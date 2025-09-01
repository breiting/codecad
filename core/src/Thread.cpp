#include "mech/Thread.hpp"

#include <algorithm>
#include <stdexcept>

// OCCT
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <Geom2d_Line.hxx>
#include <GeomAPI.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Surface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <gp.hxx>
#include <gp_Ax3.hxx>

namespace mech {

namespace {

// Fundamental triangle height H for 60° profile
inline double FundamentalHeight(double pitch) {
    return 0.86602540378443864676 * pitch;  // sqrt(3)/2 * P
}

/**
 * @brief Create a 3D helix as a BSpline curve (no To3d / surface mapping needed).
 *
 * Parametrization:
 *   t in [0, 2π * turns]
 *   x = r * cos(t)
 *   y = r * sin(t)
 *   z = s * t,  s = ± pitch / (2π)
 */
Handle(Geom_Curve) MakeHelixCurve(double radius, double pitch, double length, bool leftHand) {
    if (radius <= 0.0 || pitch <= 0.0 || length <= 0.0) {
        throw std::invalid_argument("MakeHelixCurve: radius, pitch, length must be > 0");
    }

    const double turns = length / pitch;
    const double t0 = 0.0;
    const double t1 = 2.0 * M_PI * turns;
    const int samples = std::max(32, (int)std::ceil(64.0 * turns));  // adaptive sampling
    const double dt = (t1 - t0) / samples;
    const double slope = (leftHand ? -1.0 : +1.0) * pitch / (2.0 * M_PI);

    TColgp_Array1OfPnt pts(1, samples + 1);
    for (int i = 0; i <= samples; ++i) {
        const double t = t0 + i * dt;
        const double x = radius * std::cos(t);
        const double y = radius * std::sin(t);
        const double z = slope * t;
        pts.SetValue(i + 1, gp_Pnt(x, y, z));
    }

    GeomAPI_PointsToBSpline interp(pts, 3 /*Degree*/, 8 /*NbPoints*/, GeomAbs_C2, 1.0e-6);
    Handle(Geom_BSplineCurve) spline = interp.Curve();
    return spline;
}

TopoDS_Wire MakeHelixWire(double radius, double pitch, double length, bool leftHand) {
    Handle(Geom_Curve) helix = MakeHelixCurve(radius, pitch, length, leftHand);
    TopoDS_Edge e = BRepBuilderAPI_MakeEdge(helix);
    return BRepBuilderAPI_MakeWire(e);
}

/**
 * @brief Build a closed 2D trapezoid profile (one pitch wide) with 60° flanks + flats.
 *
 * We place the profile in plane Y=0, using (X,Z) where:
 *  - X runs along the unrolled circumference (0..pitch)
 *  - Z is radial (crest positive, root negative)
 */
TopoDS_Wire MakeThreadProfile2D(double pitch, const ThreadProfile& pf, bool external) {
    const double H = FundamentalHeight(pitch);
    const double fc = std::max(0.0, pf.crestFlatK * H);
    const double fr = std::max(0.0, (external ? pf.rootFlatExtK : pf.rootFlatIntK) * H);

    double depth = H - 0.5 * (fc + fr);
    if (depth <= 0.0) depth = 0.5 * H;  // fallback

    const double zTop = +0.5 * depth;
    const double zBot = -0.5 * depth;

    // Flats centered over one pitch width
    const double crestL = ((pitch - fc) * 0.5);
    const double crestR = ((pitch + fc) * 0.5);
    const double rootR = ((pitch + fr) * 0.5);
    const double rootL = ((pitch - fr) * 0.5);

    gp_Pnt p1(crestL, 0.0, zTop);
    gp_Pnt p2(crestR, 0.0, zTop);
    gp_Pnt p3(rootR, 0.0, zBot);
    gp_Pnt p4(rootL, 0.0, zBot);

    TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(p1, p2);
    TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(p2, p3);
    TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge(p3, p4);
    TopoDS_Edge e4 = BRepBuilderAPI_MakeEdge(p4, p1);

    TopoDS_Wire w = BRepBuilderAPI_MakeWire(e1, e2, e3, e4);
    return w;
}

/** @brief Sweep profile along helix via PipeShell; MakeSolid(true) to get a solid. */
TopoDS_Shape SweepThread(const TopoDS_Wire& spine, const TopoDS_Wire& section, bool makeSolid) {
    BRepOffsetAPI_MakePipeShell mk(spine);
    mk.SetMode(true /*isFrenet*/);
    mk.Add(section);
    mk.Build();
    if (makeSolid) mk.MakeSolid();
    return mk.Shape();
}

void ApplyLeadInChamfer(TopoDS_Shape& /*threadSolid*/, double /*len*/) {
    // Hook for a future conical chamfer (cut). Defaults print well without this.
}

}  // namespace

// ---- Public API ----

double IsoCoarsePitch(double dNominal) {
    if (dNominal <= 3.0)
        return 0.5;
    else if (dNominal <= 4.0)
        return 0.7;
    else if (dNominal <= 5.0)
        return 0.8;
    else if (dNominal <= 6.0)
        return 1.0;
    else if (dNominal <= 8.0)
        return 1.25;
    else if (dNominal <= 10.0)
        return 1.5;
    else if (dNominal <= 12.0)
        return 1.75;
    else if (dNominal <= 16.0)
        return 2.0;
    else if (dNominal <= 20.0)
        return 2.5;
    else if (dNominal <= 24.0)
        return 3.0;
    return 3.0;  // fallback
}

geometry::ShapePtr MakeExternalMetricThread(double dMajor, double pitch, double length, const ThreadOpts& o) {
    if (dMajor <= 0.0 || pitch <= 0.0 || length <= 0.0) {
        throw std::invalid_argument("MakeExternalMetricThread: dMajor, pitch, length must be > 0");
    }

    // For printing: slightly reduce crest radius
    const double rCrest = 0.5 * std::max(0.0, dMajor - 2.0 * o.xy);

    TopoDS_Wire helix = MakeHelixWire(rCrest, pitch, length, o.leftHand);
    TopoDS_Wire profile = MakeThreadProfile2D(pitch, o.profile, /*external*/ true);

    TopoDS_Shape vol = SweepThread(helix, profile, /*solid*/ true);

    // Core cylinder: use correct constructor (R, H)
    TopoDS_Shape core = BRepPrimAPI_MakeCylinder(rCrest, length).Shape();

    TopoDS_Shape threaded = BRepAlgoAPI_Fuse(core, vol).Shape();
    if (o.leadIn > 0.0) ApplyLeadInChamfer(threaded, o.leadIn);
    return std::make_shared<geometry::Shape>(threaded);
}

geometry::ShapePtr MakeInternalMetricThread(double dNominal, double pitch, double height, const ThreadOpts& o) {
    if (dNominal <= 0.0 || pitch <= 0.0 || height <= 0.0) {
        throw std::invalid_argument("MakeInternalMetricThread: dNominal, pitch, height must be > 0");
    }

    // For printing: enlarge nominal radius
    const double rNom = 0.5 * (dNominal + 2.0 * o.xy);

    TopoDS_Shape bore = BRepPrimAPI_MakeCylinder(rNom, height).Shape();
    TopoDS_Wire helix = MakeHelixWire(rNom, pitch, height, o.leftHand);
    TopoDS_Wire prof = MakeThreadProfile2D(pitch, o.profile, /*external*/ false);
    TopoDS_Shape vol = SweepThread(helix, prof, /*solid*/ true);

    TopoDS_Shape nut = BRepAlgoAPI_Cut(bore, vol).Shape();
    if (o.leadIn > 0.0) ApplyLeadInChamfer(nut, o.leadIn);
    return std::make_shared<geometry::Shape>(nut);
}

}  // namespace mech
