#include "mech/ThreadChamfer.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

// OCCT
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>

namespace {

// Build a revolved triangular "ring wedge" cutter for an EXTERNAL thread end.
// We construct a triangle in the XZ-plane and revolve 360° around Z.
static TopoDS_Shape BuildExternalChamferCutter(double outerR, double chamferLenZ, double angleDeg) {
    if (outerR <= 0.0 || chamferLenZ <= 0.0 || angleDeg <= 0.0) {
        throw std::invalid_argument("BuildExternalChamferCutter: invalid inputs");
    }

    // Radial drop across chamfer (along Z): dr = tan(angle) * lengthZ
    const double dr = std::tan(angleDeg * M_PI / 180.0) * chamferLenZ;
    // Ensure we don't invert the triangle if dr > outerR
    const double rInner = std::max(0.0, outerR - dr);

    // Triangle in XZ plane (Y=0):
    // P0: (outerR, 0)      -- lies on the end face
    // P1: (outerR, +Lz)    -- move along +Z, same radius
    // P2: (rInner, +Lz)    -- inner radius at the end of chamfer
    gp_Pnt p0(outerR, 0.0, 0.0);
    gp_Pnt p1(outerR, 0.0, chamferLenZ);
    gp_Pnt p2(rInner, 0.0, chamferLenZ);

    TopoDS_Edge e01 = BRepBuilderAPI_MakeEdge(p0, p1);
    TopoDS_Edge e12 = BRepBuilderAPI_MakeEdge(p1, p2);
    TopoDS_Edge e20 = BRepBuilderAPI_MakeEdge(p2, p0);

    BRepBuilderAPI_MakeWire mw;
    mw.Add(e01);
    mw.Add(e12);
    mw.Add(e20);
    TopoDS_Wire triWire = mw.Wire();

    // Make face from triangle
    TopoDS_Face triFace = BRepBuilderAPI_MakeFace(triWire);

    // Revolve around Z axis (full 2π) to get a conical ring segment
    gp_Ax1 axis(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
    // BRepPrimAPI_MakeRevol(face, axis, angle, copy)
    BRepPrimAPI_MakeRevol revol(triFace, axis, 2.0 * M_PI, true);
    TopoDS_Shape cutter = revol.Shape();

    return cutter;
}

static TopoDS_Shape BuildInternalChamferCutter(double innerR, double chamferLenZ, double angleDeg) {
    if (innerR <= 0.0 || chamferLenZ <= 0.0 || angleDeg <= 0.0)
        throw std::invalid_argument("BuildInternalChamferCutter: invalid inputs");

    // Radial increase across chamfer (going into the part)
    const double dr = std::tan(angleDeg * M_PI / 180.0) * chamferLenZ;

    // Triangle in XZ (Y=0):
    // P0: (innerR, 0)          at entry plane
    // P1: (innerR, +Lz)        same radius, deeper by Lz
    // P2: (innerR + dr, +Lz)   larger radius at Lz → creates opening
    gp_Pnt p0(innerR, 0.0, 0.0);
    gp_Pnt p1(innerR, 0.0, chamferLenZ);
    gp_Pnt p2(innerR + dr, 0.0, chamferLenZ);

    TopoDS_Edge e01 = BRepBuilderAPI_MakeEdge(p0, p1);
    TopoDS_Edge e12 = BRepBuilderAPI_MakeEdge(p1, p2);
    TopoDS_Edge e20 = BRepBuilderAPI_MakeEdge(p2, p0);

    BRepBuilderAPI_MakeWire mw;
    mw.Add(e01);
    mw.Add(e12);
    mw.Add(e20);
    TopoDS_Wire tri = mw.Wire();

    TopoDS_Face face = BRepBuilderAPI_MakeFace(tri);
    gp_Ax1 axis(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
    return BRepPrimAPI_MakeRevol(face, axis, 2.0 * M_PI, true).Shape();
}

// Apply CUT with a cutter positioned at the start end (z near 0)
static TopoDS_Shape CutStart(const TopoDS_Shape& solid, const TopoDS_Shape& baseCutter) {
    // baseCutter is already located at z ∈ [0, Lz] (triangle built with z from 0..Lz)
    BRepAlgoAPI_Cut cut(solid, baseCutter);
    cut.Build();
    if (!cut.IsDone()) throw std::runtime_error("Chamfer CUT (start) failed");
    return cut.Shape();
}

// Apply CUT with a cutter positioned at the far end (z near totalLen)
static TopoDS_Shape CutEnd(const TopoDS_Shape& solid, const TopoDS_Shape& baseCutter, double totalLen,
                           double chamferLenZ) {
    // Move the cutter so that its z-span [0..Lz] sits at [totalLen - Lz .. totalLen]
    gp_Trsf t;
    t.SetTranslation(gp_Vec(0, 0, std::max(0.0, totalLen - chamferLenZ)));
    BRepBuilderAPI_Transform bt(baseCutter, t, true);
    bt.Build();
    if (!bt.IsDone()) throw std::runtime_error("Chamfer cutter transform (end) failed");
    TopoDS_Shape cutterEnd = bt.Shape();

    BRepAlgoAPI_Cut cut(solid, cutterEnd);
    cut.Build();
    if (!cut.IsDone()) throw std::runtime_error("Chamfer CUT (end) failed");
    return cut.Shape();
}

}  // namespace

namespace mech {

TopoDS_Shape ChamferThreadEndsExternal(const TopoDS_Shape& threaded, double outerRadius, double lengthZ,
                                       double angleDeg, double totalLengthZ, bool start, bool end) {
    if (lengthZ <= 0.0 || angleDeg <= 0.0 || totalLengthZ <= 0.0) {
        return threaded;  // nothing to do
    }

    // Build a single base cutter (at z from 0..lengthZ)
    TopoDS_Shape cutterBase = BuildExternalChamferCutter(outerRadius, lengthZ, angleDeg);

    TopoDS_Shape out = threaded;

    if (start) {
        out = CutStart(out, cutterBase);
    }
    if (end) {
        out = CutEnd(out, cutterBase, totalLengthZ, lengthZ);
    }

    return out;
}

TopoDS_Shape ChamferThreadEndsInternal(const TopoDS_Shape& nut, double innerRadius, double lengthZ, double angleDeg,
                                       double totalLengthZ, bool start, bool end) {
    if (lengthZ <= 0.0 || angleDeg <= 0.0 || totalLengthZ <= 0.0) return nut;

    TopoDS_Shape cutterBase = BuildInternalChamferCutter(innerRadius, lengthZ, angleDeg);

    TopoDS_Shape out = nut;
    if (start) out = CutStart(out, cutterBase);
    if (end) out = CutEnd(out, cutterBase, totalLengthZ, lengthZ);
    return out;
}

}  // namespace mech
