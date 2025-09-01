#include "mech/CoarseThread.hpp"

// std
#include <cmath>
#include <stdexcept>

// OCCT
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <Geom_BSplineCurve.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>

using geometry::Shape;
using geometry::ShapePtr;

const double TOLERANCE = 1e-5;

namespace mech {

// --- local utils ------------------------------------------------------------

static inline double ClampPositive(double v, double eps = 1e-9) {
    return (v < eps) ? eps : v;
}

static inline double DegToRad(double d) {
    return d * M_PI / 180.0;
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

TopoDS_Shape MakeCylinder(double radius, double height) {
    if (radius <= 0.0 || height <= 0.0) {
        throw std::invalid_argument("MakeCylinder: invalid radius/height");
    }
    return BRepPrimAPI_MakeCylinder(radius, height).Shape();
}

TopoDS_Shape MakeHexPrism(double acrossFlats, double height) {
    if (acrossFlats <= 0.0 || height <= 0.0) {
        throw std::invalid_argument("MakeHexPrism: invalid AF/height");
    }
    // Hex radius (circumradius) from across-flats: R = AF / (2 cos 30°)
    const double R = acrossFlats / (2.0 * std::cos(DegToRad(30.0)));

    BRepBuilderAPI_MakePolygon poly;
    for (int i = 0; i < 6; ++i) {
        const double a = DegToRad(60.0 * i);
        poly.Add(gp_Pnt(R * std::cos(a), R * std::sin(a), 0.0));
    }
    poly.Close();

    TopoDS_Wire w = poly.Wire();
    TopoDS_Face f = BRepBuilderAPI_MakeFace(w);
    return BRepPrimAPI_MakePrism(f, gp_Vec(0, 0, height)).Shape();
}

/**
 * @brief Create a C2 B-spline helix sampled from param t in [0, 2π*turns].
 *
 * Spine points: (R cos t, R sin t, ± (pitch / (2π)) * t)
 */
Handle(Geom_Curve) MakeHelixCurve(double radius, double pitch, double length, bool leftHand) {
    radius = ClampPositive(radius);
    pitch = ClampPositive(pitch);
    length = ClampPositive(length);

    const double turns = length / pitch;
    const int n = std::max(32, int(96 * turns));  // sampling density
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
TopoDS_Wire MakeVProfileYZ(double depth, double flankDeg, ThreadTip tip, double tipFlatRatio) {
    depth = ClampPositive(depth);
    flankDeg = std::max(5.0, flankDeg);

    const double half = DegToRad(0.5 * flankDeg);
    const double base = 2.0 * depth * std::tan(half);  // base width on Z

    // Base corners on Y=0
    const gp_Pnt bL(0, 0, -0.5 * base);
    const gp_Pnt bR(0, 0, +0.5 * base);

    BRepBuilderAPI_MakePolygon poly;

    if (tip == ThreadTip::Sharp) {
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

// --- public API -------------------------------------------------------------

ShapePtr CoarseThread::ThreadExternal(double outerDiameter, const CoarseThreadParams& p) {
    const double L = ClampPositive(p.length);
    const int T = std::max(1, p.turns);
    const double P = L / double(T);  // pitch
    const double Do = ClampPositive(outerDiameter - 2.0 * std::max(0.0, p.clearance));
    const double Ro = 0.5 * Do;               // major radius
    const double d = ClampPositive(p.depth);  // radial depth

    // Core = cylinder at minor radius (Ro - d)
    const double Rcore = ClampPositive(Ro - d);
    TopoDS_Shape core = MakeCylinder(Rcore, L);

    // Profile on pitch radius (roughly mid-radial thickness): Rp = Ro - d/2
    const double Rpitch = ClampPositive(Ro - 0.5 * d);

    Handle(Geom_Curve) helix = MakeHelixCurve(Rpitch, P, L, p.leftHand);
    TopoDS_Wire helixWire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(helix));

    // Profile in local YZ, then rotate/translate into world so that local +Y -> +X (radial)
    TopoDS_Wire profileYZ = MakeVProfileYZ(d, p.flankAngleDeg, p.tip, p.tipFlatRatio);

    gp_Trsf rotZ;
    rotZ.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), -M_PI / 2.0);
    gp_Trsf shift;
    shift.SetTranslation(gp_Vec(Rpitch - 0.5 * d, 0, 0));  // centroid ~ d/2 from base
    TopoDS_Wire profilePlaced = TransformWire(profileYZ, shift * rotZ);

    TopoDS_Shape ridges = SweepAlongHelix(helixWire, profilePlaced);
    TopoDS_Shape threaded = Fuse(core, ridges);
    return std::make_shared<Shape>(threaded);
}

ShapePtr CoarseThread::ThreadInternal(double boreDiameter, const CoarseThreadParams& p) {
    const double L = ClampPositive(p.length);
    const int T = std::max(1, p.turns);
    const double P = L / double(T);
    const double Di = ClampPositive(boreDiameter + 2.0 * std::max(0.0, p.clearance));
    const double Ri = 0.5 * Di;
    const double d = ClampPositive(p.depth);

    // For an internal cutter, build the profile so it points inward (towards -X after placement).
    const double Rpitch = ClampPositive(Ri + 0.5 * d);

    Handle(Geom_Curve) helix = MakeHelixCurve(Rpitch, P, L, p.leftHand);
    TopoDS_Wire helixWire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(helix));

    TopoDS_Wire profileYZ = MakeVProfileYZ(d, p.flankAngleDeg, p.tip, p.tipFlatRatio);

    // Rotate -90°: local +Y -> world -X (inwards), then shift so centroid sits at Rpitch.
    gp_Trsf rotZ;
    rotZ.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), M_PI / 2.0);
    gp_Trsf shift;
    shift.SetTranslation(gp_Vec(-(Rpitch - 0.5 * d), 0, 0));
    TopoDS_Wire profilePlaced = TransformWire(profileYZ, shift * rotZ);

    TopoDS_Shape cutter = SweepAlongHelix(helixWire, profilePlaced);
    return std::make_shared<Shape>(cutter);  // subtract this from your part
}

ShapePtr CoarseThread::MakeBolt(double outerDiameter, double totalLength, double headAcrossFlats, double headHeight,
                                const CoarseThreadParams& p) {
    if (totalLength <= 0.0 || headAcrossFlats <= 0.0 || headHeight <= 0.0) {
        throw std::invalid_argument("MakeBolt: invalid dimensions");
    }
    if (p.length <= 0.0 || p.turns <= 0) {
        throw std::invalid_argument("MakeBolt: thread length/turns must be > 0");
    }
    if (p.length > totalLength) {
        throw std::invalid_argument("MakeBolt: thread length must be <= totalLength");
    }

    const double Rmaj = 0.5 * ClampPositive(outerDiameter);
    const double Lthread = p.length;
    const double Lshank = totalLength - Lthread;

    // Optional shank (straight, major radius)
    TopoDS_Shape stem;
    if (Lshank > 1e-9) {
        stem = MakeCylinder(Rmaj, Lshank);
    }

    // External thread
    TopoDS_Shape threaded = ThreadExternal(outerDiameter, p)->Get();
    if (Lshank > 1e-9) {
        // move threaded part above shank
        gp_Trsf up;
        up.SetTranslation(gp_Vec(0, 0, Lshank));
        threaded = BRepBuilderAPI_Transform(threaded, up, true).Shape();
    }

    TopoDS_Shape rod = threaded;
    if (!stem.IsNull()) rod = Fuse(stem, threaded);

    // Hex head below z=0
    TopoDS_Shape head = MakeHexPrism(headAcrossFlats, headHeight);
    gp_Trsf down;
    down.SetTranslation(gp_Vec(0, 0, -headHeight));
    head = BRepBuilderAPI_Transform(head, down, true).Shape();

    return std::make_shared<Shape>(rod);  // Fuse(rod, head));
}

ShapePtr CoarseThread::MakeNut(double boreDiameter, double thickness, double acrossFlats,
                               const CoarseThreadParams& pIn) {
    if (boreDiameter <= 0.0 || thickness <= 0.0 || acrossFlats <= 0.0) {
        throw std::invalid_argument("MakeNut: invalid dimensions");
    }

    // Hex body
    TopoDS_Shape hex = MakeHexPrism(acrossFlats, thickness);

    // Straight bore
    const double Rbore = 0.5 * ClampPositive(boreDiameter);
    TopoDS_Shape bore = MakeCylinder(Rbore, thickness);
    TopoDS_Shape nut = Cut(hex, bore);

    // Internal cutter (full length)
    CoarseThreadParams p = pIn;
    p.length = thickness;
    TopoDS_Shape cutter = ThreadInternal(boreDiameter, p)->Get();

    // return std::make_shared<Shape>(Cut(nut, cutter));
    return std::make_shared<Shape>(cutter);
}

geometry::ShapePtr CoarseThread::Test(const CoarseThreadParams& p) {
    const double bodyOuterR = 30.0;  // Außenradius des Bauteils
    TopoDS_Shape body = BRepPrimAPI_MakeCylinder(bodyOuterR, 100).Shape();

    // Innengewinde-Parameter (identisch zu außen – außer Länge)
    double threadLen_I = 30.0;
    double depth = 3.5;
    double clearance = 0.15;
    double outerD = 24.0;
    CoarseThreadParams pI{};
    pI.length = threadLen_I;
    pI.turns = 2;
    pI.depth = depth;
    pI.clearance = clearance;  // leicht positives Spiel
    pI.flankAngleDeg = 60.0;
    pI.leftHand = false;
    pI.tip = ThreadTip::Cut;
    pI.tipFlatRatio = 0.5;

    // Minor-Durchmesser ~ outerD - 2*depth (plus minimaler Sicherheitsabzug)
    const double minorD = std::max(1e-3, outerD - 2.0 * depth);

    TopoDS_Shape bore = BRepPrimAPI_MakeCylinder(0.5 * minorD, threadLen_I).Shape();
    BRepAlgoAPI_Cut opBore(body, bore);
    opBore.SetFuzzyValue(1.0e-5);
    opBore.Build();
    if (!opBore.IsDone()) throw std::runtime_error("Pre-bore boolean failed");
    body = opBore.Shape();

    // (b) Gewindecutter generieren (mit gleichen Parametern, gleicher Steigung)
    auto cutterPtr = CoarseThread::ThreadInternal(minorD, pI);
    const TopoDS_Shape cutter = cutterPtr->Get();

    // Cutter in den Körper schneiden
    BRepAlgoAPI_Cut opCut(body, cutter);
    opCut.SetFuzzyValue(2.0e-5);  // etwas größerer Fuzzy-Wert für robuste Schnitte
    opCut.Build();
    if (!opCut.IsDone()) throw std::runtime_error("Thread cut boolean failed");
    TopoDS_Shape bodyThreaded = opCut.Shape();

    gp_Trsf up;
    up.SetTranslation(gp_Vec(50, 0, 0));
    bodyThreaded = BRepBuilderAPI_Transform(bodyThreaded, up, true).Shape();

    return std::make_shared<Shape>(cutter);
    // return std::make_shared<Shape>(bodyThreaded);
}

}  // namespace mech
