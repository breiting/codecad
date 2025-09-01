#include "mech/CoarseThread.hpp"

// Standard
#include <cmath>
#include <stdexcept>

// OCCT
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <Geom2d_Line.hxx>
#include <GeomAPI.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Surface.hxx>
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

#include "geometry/Shape.hpp"

namespace mech {

// --- small helpers ----------------------------------------------------------

static double clampPositive(double v, double eps = 1e-6) {
    return (v < eps) ? eps : v;
}

static Handle(Geom_Curve) makeHelixCurve(double radius, double pitch, double length, bool leftHand) {
    if (radius <= 0.0 || pitch <= 0.0 || length <= 0.0) {
        throw std::invalid_argument("makeHelixCurve: radius, pitch, length must be > 0");
    }

    const double turns = length / pitch;
    const int total_points = std::max(16, static_cast<int>(96 * turns));  // TODO: 96
    const double t_max = 2.0 * M_PI * turns;
    const double dt = t_max / (total_points - 1);
    const double z_step = (leftHand ? -1.0 : 1.0) * pitch / (2.0 * M_PI);

    TColgp_Array1OfPnt points(1, total_points);

    for (int i = 0; i < total_points; ++i) {
        const double t = i * dt;
        const double x = radius * std::cos(t);
        const double y = radius * std::sin(t);
        const double z = z_step * t;

        points.SetValue(i + 1, gp_Pnt(x, y, z));
    }

    // Create B-spline with appropriate degree and smoothness
    GeomAPI_PointsToBSpline spline_builder(points,
                                           3,           // degree
                                           8,           // max degree
                                           GeomAbs_C2,  // continuity
                                           1.0e-6       // tolerance
    );

    if (!spline_builder.IsDone()) {
        throw std::runtime_error("Failed to create helix B-spline");
    }

    return spline_builder.Curve();
}

/**
 * @brief Make a simple isosceles V-profile (wire) in the local YZ-plane.
 *
 * - Radial direction = +Y (outwards)
 * - Axial direction  = +Z
 * - Triangle height (radial) = depth
 * - Base width = 2 * depth * tan(flankAngle/2)
 */
static TopoDS_Wire makeVProfileYZ(double depth, double flankAngleDeg) {
    const double a = std::max(5.0, flankAngleDeg);  // sanity
    const double half = 0.5 * (a * M_PI / 180.0);
    const double base = 2.0 * depth * std::tan(half);

    // Tip at (0, depth, 0), base centered at z=0
    gp_Pnt p_tip(0, depth, 0);
    gp_Pnt p_bL(0, 0, -0.5 * base);
    gp_Pnt p_bR(0, 0, 0.5 * base);

    TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(p_tip, p_bL);
    TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(p_bL, p_bR);
    TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge(p_bR, p_tip);

    BRepBuilderAPI_MakeWire mw;
    mw.Add(e1);
    mw.Add(e2);
    mw.Add(e3);
    return mw.Wire();
}

static TopoDS_Wire transformWire(const TopoDS_Wire& w, const gp_Trsf& tr) {
    BRepBuilderAPI_Transform bt(w, tr, /*copy*/ true);
    bt.Build();
    if (!bt.IsDone()) throw std::runtime_error("Failed to transform wire");
    return TopoDS::Wire(bt.Shape());
}

static TopoDS_Shape sweepAlongHelix(const TopoDS_Wire& helix, const TopoDS_Wire& profilePlaced, bool frenet) {
    BRepOffsetAPI_MakePipeShell pipe(helix);
    pipe.SetMode(frenet);
    pipe.Add(profilePlaced);
    pipe.Build();
    if (!pipe.IsDone()) throw std::runtime_error("Pipe shell build failed");
    pipe.MakeSolid();
    return pipe.Shape();
}

// --- core implementation -----------------------------------------------------

geometry::ShapePtr CoarseThread::External(double outerDiameter, const CoarseThreadParams& p) {
    if (p.turns < 1) throw std::invalid_argument("turns must be >= 1");
    const double L = clampPositive(p.length);
    const double d = clampPositive(p.depth);
    const double t = static_cast<double>(p.turns);
    const double P = L / t;  // pitch
    const double Do = clampPositive(outerDiameter - 2.0 * std::max(0.0, p.clearance));
    const double Ro = 0.5 * Do;

    // Core radius = outer - depth
    const double Rcore = clampPositive(Ro - d);
    // Place profile center on pitch radius ~ Ro - d/2
    const double Rpitch = clampPositive(Ro - 0.5 * d);

    // Build helix on Rpitch
    Handle(Geom_Curve) helix = makeHelixCurve(Rpitch, P, L, p.leftHand);
    TopoDS_Wire helixWire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(helix));

    // Build V-profile in local YZ (radial +Y, axial +Z)
    TopoDS_Wire profileYZ = makeVProfileYZ(d, p.flankAngleDeg);

    // Rotate +90° around Z so local +Y -> world +X (radial)
    gp_Trsf rotZ;
    rotZ.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), +M_PI / 2.0);
    // Shift so that profile center sits at Rpitch (its centroid is at d/2 radially)
    gp_Trsf shift;
    shift.SetTranslation(gp_Vec(Rpitch - 0.5 * d, 0, 0));
    gp_Trsf place = shift * rotZ;

    TopoDS_Wire profilePlaced = transformWire(profileYZ, place);

    // Sweep to ridges
    TopoDS_Shape ridges = sweepAlongHelix(helixWire, profilePlaced, /*frenet*/ true);

    // Core solid
    TopoDS_Shape core = BRepPrimAPI_MakeCylinder(Rcore, L).Shape();

    // Fuse ridges to core => external thread
    TopoDS_Shape result = BRepAlgoAPI_Fuse(core, ridges).Shape();
    return std::make_shared<geometry::Shape>(ridges);
}

geometry::ShapePtr CoarseThread::InternalCutter(double boreDiameter, const CoarseThreadParams& p) {
    if (p.turns < 1) throw std::invalid_argument("turns must be >= 1");
    const double L = clampPositive(p.length);
    const double d = clampPositive(p.depth);
    const double t = static_cast<double>(p.turns);
    const double P = L / t;  // pitch
    const double Di = clampPositive(boreDiameter + 2.0 * std::max(0.0, p.clearance));
    const double Ri = 0.5 * Di;

    // For an internal cutter, we want the HELIX around the pitch radius just OUTSIDE the bore:
    // Rpitch = Ri + d/2. The profile must point INWARDS (towards -X after placement).
    const double Rpitch = clampPositive(Ri + 0.5 * d);

    // Helix
    Handle(Geom_Curve) helix = makeHelixCurve(Rpitch, P, L, p.leftHand);
    TopoDS_Wire helixWire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(helix));

    // Build V-profile in local YZ (radial +Y outward)
    TopoDS_Wire profileYZ = makeVProfileYZ(d, p.flankAngleDeg);

    // For inward cutter: rotate -90° so local +Y -> world -X (pointing inward),
    // then shift to Rpitch (centroid at d/2).
    gp_Trsf rotZ;
    rotZ.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), -M_PI / 2.0);
    gp_Trsf shift;
    shift.SetTranslation(gp_Vec(-(Rpitch - 0.5 * d), 0, 0));  // negative X shift
    gp_Trsf place = shift * rotZ;

    TopoDS_Wire profilePlaced = transformWire(profileYZ, place);

    TopoDS_Shape cyl = BRepPrimAPI_MakeCylinder(6, 10).Shape();

    TopoDS_Shape box = BRepPrimAPI_MakeBox(20, 20, 10).Shape();
    gp_Trsf tr;
    tr.SetTranslation(gp_Vec(-10, -10, 0));
    TopoDS_Shape out = BRepBuilderAPI_Transform(box, tr, true).Shape();

    // Sweep -> this is the CUTTER you subtract from your nut/part
    TopoDS_Shape cutter = sweepAlongHelix(helixWire, profilePlaced, /*frenet*/ true);

    auto remove = BRepAlgoAPI_Fuse(cyl, cutter);

    auto result = BRepAlgoAPI_Cut(out, remove);

    return std::make_shared<geometry::Shape>(result);
}

}  // namespace mech
