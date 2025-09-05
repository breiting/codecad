#include "ccad/mech/Threads.hpp"

#include <sys/types.h>

#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <Geom_BSplineCurve.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <algorithm>
#include <cmath>
#include <gp.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax3.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Trsf.hxx>
#include <map>
#include <stdexcept>

#include "internal/geom/ShapeHelper.hpp"

const double TOLERANCE = 1e-3;

namespace ccad {
namespace mech {

// --- small helpers ----------------------------------------------------------

static inline double ClampPositive(double v, double eps = 1e-9) {
    return (v < eps) ? eps : v;
}

static inline double DegToRad(double d) {
    return d * M_PI / 180.0;
}

static inline double CalculateBase(double flankAngleDeg, double depth) {
    const double half = DegToRad(0.5 * flankAngleDeg);
    return 2.0 * depth * std::tan(half);
}

TopoDS_Shape Fuse(const TopoDS_Shape& a, const TopoDS_Shape& b) {
    BRepAlgoAPI_Fuse op(a, b);
    op.SetRunParallel(true);
    op.SetFuzzyValue(TOLERANCE);
    op.Build();
    if (!op.IsDone()) {
        op.DumpErrors(std::cerr);
        throw std::runtime_error("Fuse failed");
    }
    return op.Shape();
}

TopoDS_Shape Cut(const TopoDS_Shape& a, const TopoDS_Shape& b) {
    BRepAlgoAPI_Cut op(a, b);
    op.SetRunParallel(true);
    op.SetFuzzyValue(TOLERANCE);
    if (!op.IsDone()) throw std::runtime_error("Cut failed");
    return op.Shape();
}

void PrintWireCoordinates(const TopoDS_Wire& wire) {
    TopExp_Explorer exp(wire, TopAbs_VERTEX);
    for (int i = 0; exp.More(); exp.Next(), ++i) {
        TopoDS_Vertex v = TopoDS::Vertex(exp.Current());
        gp_Pnt p = BRep_Tool::Pnt(v);

        printf("Vertex %d: X = %.6f, Y = %.6f, Z = %.6f\n", i, p.X(), p.Y(), p.Z());
    }
}

namespace iso {
/**
 * @brief Standard metric thread pitch table (ISO 262).
 *
 * Provides standard coarse and fine pitches for common metric thread sizes.
 * Based on ISO 262 standard.
 */
const std::map<double, std::vector<double>> METRIC_PITCHES = {{1.0, {0.25}},
                                                              {1.2, {0.25}},
                                                              {1.6, {0.35}},
                                                              {2.0, {0.4, 0.25}},
                                                              {2.5, {0.45, 0.35}},
                                                              {3.0, {0.5, 0.35}},
                                                              {4.0, {0.7, 0.5}},
                                                              {5.0, {0.8, 0.5}},
                                                              {6.0, {1.0, 0.75, 0.5}},
                                                              {8.0, {1.25, 1.0, 0.75}},
                                                              {10.0, {1.5, 1.25, 1.0, 0.75}},
                                                              {12.0, {1.75, 1.5, 1.25, 1.0}},
                                                              {16.0, {2.0, 1.5, 1.0}},
                                                              {20.0, {2.5, 2.0, 1.5, 1.0}},
                                                              {24.0, {3.0, 2.5, 2.0, 1.5}},
                                                              {30.0, {3.5, 3.0, 2.0, 1.5}},
                                                              {36.0, {4.0, 3.0, 2.0, 1.5}},
                                                              {42.0, {4.5, 4.0, 3.0, 2.0}},
                                                              {48.0, {5.0, 4.0, 3.0, 2.0}}};

double MetricStandard::GetCoarsePitch(double nominalDiameter) {
    auto it = METRIC_PITCHES.find(nominalDiameter);
    if (it != METRIC_PITCHES.end() && !it->second.empty()) {
        return it->second[0];  // First entry is coarse pitch
    }

    // Fallback calculation for non-standard sizes
    if (nominalDiameter <= 2.0) return 0.4;
    if (nominalDiameter <= 3.0) return 0.5;
    if (nominalDiameter <= 5.0) return 0.8;
    if (nominalDiameter <= 8.0) return 1.25;
    if (nominalDiameter <= 12.0) return 1.75;
    if (nominalDiameter <= 18.0) return 2.5;
    if (nominalDiameter <= 24.0) return 3.0;

    return 3.5;  // Default for large threads
}

std::vector<double> MetricStandard::GetFinePitches(double nominalDiameter) {
    auto it = METRIC_PITCHES.find(nominalDiameter);
    if (it != METRIC_PITCHES.end() && it->second.size() > 1) {
        return std::vector<double>(it->second.begin() + 1, it->second.end());
    }
    return {};  // No fine pitches available
}

}  // namespace iso

/**
 * @brief Create a C2 B-spline helix sampled from param t in [0, 2π*turns].
 *
 * Spine points: (R cos t, R sin t, ± (pitch / (2π)) * t)
 */
Handle(Geom_Curve) MakeHelixCurve(double radius, double pitch, double length, bool leftHand, int segmentsPerTurn) {
    radius = ClampPositive(radius);
    pitch = ClampPositive(pitch);
    length = ClampPositive(length);

    const double turns = length / pitch;
    const int n = std::max(32, int(segmentsPerTurn * turns));  // sampling density
    const double tmax = 2.0 * M_PI * turns;
    const double dt = tmax / (n - 1);
    const double zstep = (leftHand ? -1.0 : +1.0) * pitch / (2.0 * M_PI);

    TColgp_Array1OfPnt pts(1, n);
    for (int i = 0; i < n; ++i) {
        const double t = i * dt;
        const double x = radius * std::cos(t);
        const double y = radius * std::sin(t);
        const double z = zstep * t;
        pts.SetValue(i + 1, gp_Pnt(x, y, z));
    }

    GeomAPI_PointsToBSpline builder(pts, 3, 8, GeomAbs_C2, 1.0e-6);  // TODO: maybe skip parameters here?
    if (!builder.IsDone()) throw std::runtime_error("MakeHelixCurve: BSpline build failed");
    return builder.Curve();
}

/**
 * @brief Make a V-profile wire in local YZ, apex at (0, depth, 0), base on Y=0.
 *        If tip==Cut, the crest is truncated by a flat of size (tipFlatRatio * depth).
 */
TopoDS_Wire MakeVProfileYZ(double depth, double flankDeg, TipStyle tip, double tipFlatRatio) {
    depth = ClampPositive(depth);
    flankDeg = std::max(5.0, flankDeg);

    const double base = CalculateBase(flankDeg, depth);

    // Base corners on Y=0
    const gp_Pnt bL(0, 0, -0.5 * base);
    const gp_Pnt bR(0, 0, +0.5 * base);

    BRepBuilderAPI_MakePolygon poly;

    if (tip == TipStyle::Sharp) {
        // Sharp apex at Y=depth, Z=0
        const gp_Pnt apex(0, depth, 0);
        poly.Add(bL);
        poly.Add(bR);
        poly.Add(apex);
        poly.Close();
    } else {
        // Truncated flat at Y = depth - flat
        const double flat = std::clamp(tipFlatRatio, 0.0, 1.0) * depth;
        const double yTop = std::max(1e-6, depth - flat);

        // The flat is parallel to the base (on Y=yTop), centered on Z=0.
        // We keep the side edges straight (same base corners).
        // Choose a conservative flat width along Z: shrink by same fraction of base.
        const double flatZ = 0.5 * base * (1.0 - tipFlatRatio);
        const gp_Pnt tL(0, yTop, -flatZ);
        const gp_Pnt tR(0, yTop, +flatZ);

        poly.Add(bL);
        poly.Add(bR);
        poly.Add(tR);
        poly.Add(tL);
        poly.Close();
    }

    return poly.Wire();
}

TopoDS_Wire TransformWire(const TopoDS_Wire& w, const gp_Trsf& tr) {
    BRepBuilderAPI_Transform bt(w, tr, /*copy*/ true);
    bt.Build();
    if (!bt.IsDone()) throw std::runtime_error("TransformWire failed");
    return TopoDS::Wire(bt.Shape());
}

TopoDS_Shape SweepAlongHelix(const TopoDS_Wire& spine, const TopoDS_Wire& section) {
    BRepOffsetAPI_MakePipeShell pipe(spine);
    pipe.SetMode(/*isFrenet*/ true);
    pipe.Add(section);
    pipe.Build();
    if (!pipe.IsDone()) throw std::runtime_error("PipeShell build failed");
    pipe.MakeSolid();
    return pipe.Shape();
}

static bool IsLeft(const ThreadSpec& s) {
    return s.handedness == Handedness::Left;
}

// ---------------- public API ----------------

Shape ThreadOps::ThreadExternalRod(const ThreadSpec& inSpec, double rodLength, double threadLength,
                                   double& majorDiameter) {
    ThreadSpec spec = inSpec;
    spec.Normalize();

    if (rodLength <= 0.0 || threadLength <= 0.0 || threadLength > rodLength)
        throw std::invalid_argument("ThreadExternalRod: invalid lengths");

    const double L = threadLength;

    const double halfDepth = 0.5 * spec.depth;
    const double R_pitch = 0.5 * spec.fitDiameter + halfDepth - spec.clearance;

    const double eps = std::max({1e-3, 0.002 * spec.pitch, 0.05 * spec.depth});

    // Base rod (full rodLength)
    const double R_minor = 0.5 * spec.fitDiameter - spec.clearance + eps;
    majorDiameter = (R_minor + spec.depth) * 2.0;
    TopoDS_Shape rod = BRepPrimAPI_MakeCylinder(R_minor, rodLength).Shape();

    // Helix for threaded section (0..L)
    Handle(Geom_Curve) helix = MakeHelixCurve(R_pitch, spec.pitch, L, IsLeft(spec), spec.segmentsPerTurn);
    TopoDS_Wire helixWire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(helix));

    // Profile in local YZ, then rotate/translate into world so that local +Y -> +X (radial)
    TopoDS_Wire profileYZ = MakeVProfileYZ(spec.depth, spec.flankAngleDeg, spec.tip, spec.tipCutRatio);

    gp_Trsf rotZ;
    rotZ.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), -M_PI / 2.0);
    gp_Trsf shift;
    const double base = CalculateBase(spec.flankAngleDeg, spec.depth);

    shift.SetTranslation(gp_Vec(R_pitch - 0.5 * spec.depth, 0, -base * 0.5));
    TopoDS_Wire profilePlaced = TransformWire(profileYZ, shift * rotZ);

    // Sweep → ridge volume for 0..L
    TopoDS_Shape ridges = SweepAlongHelix(helixWire, profilePlaced);

    // Cut top and bottom profile, because it is overhanging
    const double big = std::max({R_minor, spec.depth}) * 4.0 + 10.0;
    TopoDS_Shape clip = BRepPrimAPI_MakeBox(gp_Pnt(-big, -big, 0), gp_Pnt(+big, +big, L)).Shape();

    // Intersection (Common) statt Cut/Fuse:
    TopoDS_Shape ridgesClipped = BRepAlgoAPI_Common(ridges, clip).Shape();

    // Fuse ridges only onto the threaded section: we fuse full rods; simple and robust
    TopoDS_Shape threaded = Fuse(rod, ridgesClipped);

    return WrapOcctShape(threaded);
}

Shape ThreadOps::ThreadInternalCutter(const ThreadSpec& inSpec, double threadLength, double& boreHoleDiameter) {
    ThreadSpec spec = inSpec;
    spec.Normalize();

    if (threadLength <= 0.0) {
        throw std::invalid_argument("ThreadInternalCutter: invalid threadLength");
    }

    const double L = threadLength;

    const double halfDepth = 0.5 * spec.depth;
    const double R_pitch = 0.5 * spec.fitDiameter + halfDepth;
    const double eps = std::max({1e-3, 0.002 * spec.pitch, 0.05 * spec.depth});

    // Make sure to return proper cut shape, depending on the Tip form!
    boreHoleDiameter = spec.fitDiameter;
    if (spec.tip == TipStyle::Cut) {
        boreHoleDiameter += spec.tipCutRatio * spec.depth * 2.0;
    }

    Handle(Geom_Curve) helix = MakeHelixCurve(R_pitch, spec.pitch, L, IsLeft(spec), spec.segmentsPerTurn);
    TopoDS_Wire helixWire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(helix));

    TopoDS_Wire profileYZ = MakeVProfileYZ(spec.depth, spec.flankAngleDeg, spec.tip, spec.tipCutRatio);

    gp_Trsf rotZ;
    rotZ.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), M_PI / 2.0);
    gp_Trsf shift;
    const double base = CalculateBase(spec.flankAngleDeg, spec.depth);
    shift.SetTranslation(
        gp_Vec(-R_pitch + spec.tipCutRatio * spec.depth * 0.5 - eps, 0, spec.pitch * 0.5 - base * 0.5));
    TopoDS_Wire profilePlaced = TransformWire(profileYZ, shift * rotZ);

    TopoDS_Shape cutter = SweepAlongHelix(helixWire, profilePlaced);

    return WrapOcctShape(cutter);
}
}  // namespace mech
}  // namespace ccad
