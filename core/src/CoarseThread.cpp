#include "mech/CoarseThread.hpp"

// std
#include <algorithm>
#include <cmath>
#include <stdexcept>

// OCCT
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepLib.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <Geom2d_Line.hxx>
#include <GeomAPI.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax3.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Trsf.hxx>

using geometry::Shape;
using geometry::ShapePtr;

namespace mech {

// ---------- presets ----------------------------------------------------------

ThreadPreset PresetCoarse3D() {
    return ThreadPreset{/*depth*/ 0.9,
                        /*clearance*/ 0.22,
                        /*flankAngleDeg*/ 60.0,
                        /*samplesPerTurn*/ 96,
                        /*tipStyle*/ ThreadTip::Rounded,
                        /*tipRadius*/ 0.18};
}

ThreadPreset PresetFine3D() {
    return ThreadPreset{/*depth*/ 0.6,
                        /*clearance*/ 0.18,
                        /*flankAngleDeg*/ 60.0,
                        /*samplesPerTurn*/ 80,
                        /*tipStyle*/ ThreadTip::Rounded,
                        /*tipRadius*/ 0.12};
}

// ---------- small helpers ----------------------------------------------------

static double clampPos(double v, double eps = 1e-6) {
    return (v < eps) ? eps : v;
}

/// BSpline helix with samplesPerTurn * turns points (fast, robust for 7.9)
static Handle(Geom_Curve)
    makeHelixBSpline(double radius, double pitch, double length, bool leftHand, int samplesPerTurn) {
    const double turns = length / pitch;
    const int ptsTotal = std::max(16, samplesPerTurn * std::max(1, (int)std::ceil(turns)));
    const double tMax = 2.0 * M_PI * turns;
    const double dt = tMax / (ptsTotal - 1);
    const double zStep = (leftHand ? -1.0 : +1.0) * pitch / (2.0 * M_PI);

    TColgp_Array1OfPnt pts(1, ptsTotal);
    for (int i = 0; i < ptsTotal; ++i) {
        const double t = i * dt;
        pts.SetValue(i + 1, gp_Pnt(radius * std::cos(t), radius * std::sin(t), zStep * t));
    }

    GeomAPI_PointsToBSpline builder(pts);  //, /*DegMin*/ 3, /*DegMax*/ 45, GeomAbs_C2, 1.0e-6);
    if (!builder.IsDone()) throw std::runtime_error("makeHelixBSpline: BSpline build failed");
    return builder.Curve();
}

/// Build a V profile in local YZ with optional rounded tip (fillet radius r).
/// - Radial = +Y, axial = +Z
static TopoDS_Wire makeVProfileYZ(double depth, double flankAngleDeg, ThreadTip tip, double rFillet) {
    const double a = std::clamp(flankAngleDeg, 30.0, 100.0);
    const double half = 0.5 * a * M_PI / 180.0;
    const double base = 2.0 * depth * std::tan(half);

    // Tip at (0, depth, 0); base centered at z=0 on Y=0
    gp_Pnt tipP(0, depth, 0);
    gp_Pnt bL(0, 0, -0.5 * base);
    gp_Pnt bR(0, 0, 0.5 * base);

    // Flank direction unit vectors from tip towards bases (in YZ plane)
    const gp_Vec tipToL(bL.XYZ() - tipP.XYZ());
    const gp_Vec tipToR(bR.XYZ() - tipP.XYZ());
    gp_Vec vL = tipToL;
    vL.Normalize();
    gp_Vec vR = tipToR;
    vR.Normalize();

    TopoDS_Edge eL, eR, eArc;
    if (tip == ThreadTip::Rounded && rFillet > 1e-6) {
        // Distance from tip to tangent points on flanks: t = r / tan(half)
        const double t = rFillet / std::tan(half);
        const gp_Pnt pL = gp_Pnt(tipP.XYZ() + vL.XYZ() * t);
        const gp_Pnt pR = gp_Pnt(tipP.XYZ() + vR.XYZ() * t);

        // Lines from bases to the tangent points
        eL = BRepBuilderAPI_MakeEdge(bL, pL);
        eR = BRepBuilderAPI_MakeEdge(pR, bR);

        // Arc tangent to both flanks at pL and pR.
        // Use tangent vectors along flanks at those points (same directions as vL/vR).
        GC_MakeArcOfCircle mkArc(pL, gp_Vec(vL), pR);  // tangent at pL and pR
        if (!mkArc.IsDone()) throw std::runtime_error("Rounded tip arc failed");
        eArc = BRepBuilderAPI_MakeEdge(mkArc.Value());
    } else {
        // Sharp triangle
        eL = BRepBuilderAPI_MakeEdge(tipP, bL);
        eR = BRepBuilderAPI_MakeEdge(bR, tipP);
        // base edge
        eArc = BRepBuilderAPI_MakeEdge(bL, bR);
        BRepBuilderAPI_MakeWire mw;
        mw.Add(eL);
        mw.Add(eArc);
        mw.Add(eR);
        return mw.Wire();
    }

    // Base edge:
    TopoDS_Edge eBase = BRepBuilderAPI_MakeEdge(bL, bR);

    BRepBuilderAPI_MakeWire mw;
    mw.Add(eL);
    mw.Add(eArc);
    mw.Add(eR);
    mw.Add(eBase);
    return mw.Wire();
}

static TopoDS_Wire transformWire(const TopoDS_Wire& w, const gp_Trsf& tr) {
    BRepBuilderAPI_Transform bt(w, tr, /*copy*/ true);
    bt.Build();
    if (!bt.IsDone()) throw std::runtime_error("transformWire failed");
    return TopoDS::Wire(bt.Shape());
}

static TopoDS_Shape sweepAlong(const TopoDS_Wire& spine, const TopoDS_Wire& profilePlaced, bool frenet) {
    BRepOffsetAPI_MakePipeShell pipe(spine);
    pipe.SetMode(frenet);
    pipe.Add(profilePlaced);
    pipe.Build();
    if (!pipe.IsDone()) throw std::runtime_error("PipeShell build failed");
    pipe.MakeSolid();
    TopoDS_Shape s = pipe.Shape();
    BRepLib::SameParameter(s, 1.0e-4, true);
    return s;
}

/// Guard: keep depth/pitch relationship in a printable, robust range.
static void enforceDepthGuard(CoarseThreadParams& p) {
    const double pitch = p.length / std::max(1, p.turns);
    if (pitch <= 0) throw std::invalid_argument("pitch <= 0");
    const double maxDepth = p.maxDepthToPitch * pitch;
    if (p.depth > maxDepth) p.depth = maxDepth;
}

// ---------- public API -------------------------------------------------------

ShapePtr ThreadExternal(double outerDiameter, const CoarseThreadParams& in) {
    CoarseThreadParams p = in;
    p.length = clampPos(p.length);
    p.depth = clampPos(p.depth);
    p.turns = std::max(1, p.turns);
    p.clearance = std::max(0.0, p.clearance);
    p.samplesPerTurn = std::clamp(p.samplesPerTurn, 48, 160);
    enforceDepthGuard(p);

    const double pitch = p.length / p.turns;

    // outer / core / pitch radii (clearance reduces OD)
    const double Do = clampPos(outerDiameter - 2.0 * p.clearance);
    const double Ro = 0.5 * Do;
    const double Rcore = clampPos(Ro - p.depth);
    const double Rpitch = clampPos(Ro - 0.5 * p.depth);

    // helix on pitch radius
    Handle(Geom_Curve) helix = makeHelixBSpline(Rpitch, pitch, p.length, p.leftHand, p.samplesPerTurn);
    TopoDS_Wire spine = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(helix));

    // profile in local YZ
    TopoDS_Wire profYZ = makeVProfileYZ(p.depth, p.flankAngleDeg, p.tipStyle, p.tipRadius);

    // map +Y(radial) -> +X(world): rotation around Z with -90°, then shift so crest hits Ro
    gp_Trsf rotZ;
    rotZ.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), -M_PI / 2.0);
    gp_Trsf shift;
    shift.SetTranslation(gp_Vec(Rpitch - 0.5 * p.depth, 0, 0));
    TopoDS_Wire profPlaced = transformWire(profYZ, shift * rotZ);

    // sweep ridges and fuse with core
    TopoDS_Shape ridges = sweepAlong(spine, profPlaced, /*frenet*/ true);
    TopoDS_Shape core = BRepPrimAPI_MakeCylinder(Rcore, p.length).Shape();

    BRepAlgoAPI_Fuse fuse(core, ridges);
    fuse.SetRunParallel(true);
    fuse.SetFuzzyValue(1.0e-6);
    fuse.Build();
    if (!fuse.IsDone()) throw std::runtime_error("Fuse(core, ridges) failed");

    TopoDS_Shape result = fuse.Shape();
    return std::make_shared<Shape>(result);
}

ShapePtr ThreadInternalCutter(double boreDiameter, const CoarseThreadParams& in) {
    CoarseThreadParams p = in;
    p.length = clampPos(p.length);
    p.depth = clampPos(p.depth);
    p.turns = std::max(1, p.turns);
    p.clearance = std::max(0.0, p.clearance);
    p.samplesPerTurn = std::clamp(p.samplesPerTurn, 48, 160);
    enforceDepthGuard(p);

    const double pitch = p.length / p.turns;

    // bore (with clearance), pitch radius outside bore
    const double Di = clampPos(boreDiameter + 2.0 * p.clearance);
    const double Ri = 0.5 * Di;
    const double Rpitch = clampPos(Ri + 0.5 * p.depth);

    Handle(Geom_Curve) helix = makeHelixBSpline(Rpitch, pitch, p.length, p.leftHand, p.samplesPerTurn);
    TopoDS_Wire spine = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(helix));

    TopoDS_Wire profYZ = makeVProfileYZ(p.depth, p.flankAngleDeg, p.tipStyle, p.tipRadius);

    // inward: +Y -> -X (rotate +90°), then shift negative so crest lands at Ri
    gp_Trsf rotZ;
    rotZ.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), +M_PI / 2.0);
    gp_Trsf shift;
    shift.SetTranslation(gp_Vec(-(Rpitch - 0.5 * p.depth), 0, 0));
    TopoDS_Wire profPlaced = transformWire(profYZ, shift * rotZ);

    TopoDS_Shape cutter = sweepAlong(spine, profPlaced, /*frenet*/ true);
    return std::make_shared<Shape>(cutter);  // subtract from your part
}

}  // namespace mech
