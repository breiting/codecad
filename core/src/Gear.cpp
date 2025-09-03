#include "mech/Gear.hpp"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFilletAPI_MakeFillet2d.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepTools.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <Geom2dAPI_PointsToBSpline.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <cmath>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Pln.hxx>
#include <gp_Trsf.hxx>
#include <stdexcept>
#include <vector>

using geometry::Shape;
using geometry::ShapePtr;

namespace mech {

static inline double deg2rad(double d) {
    return d * M_PI / 180.0;
}

/// Sample involute (param t) on base circle: r_b, returns (x,y)
static inline gp_Pnt InvoluteXY(double rb, double t) {
    const double ct = std::cos(t), st = std::sin(t);
    const double x = rb * (ct + t * st);
    const double y = rb * (st - t * ct);
    return gp_Pnt(x, y, 0);
}

/// Find t for which radius == target (numeric)
static double SolveTForRadius(double rb, double targetR) {
    // radius along involute: r(t) = rb * sqrt(1 + t^2)
    // => t = sqrt( (r/ rb)^2 - 1 )
    const double ratio = targetR / rb;
    if (ratio <= 1.0) return 0.0;
    return std::sqrt(ratio * ratio - 1.0);
}

/// Make single tooth 2D face in XY plane (origin at gear center)
static TopoDS_Face MakeTooth2D(const GearSpec& g) {
    if (g.teeth < 6) throw std::invalid_argument("teeth must be >= 6");

    const double z = (double)g.teeth;
    const double m = g.module;
    const double pa = deg2rad(g.pressureDeg);
    const double rp = 0.5 * m * z;        // pitch radius
    const double ra = rp + m;             // addendum radius
    const double rf = rp - 1.25 * m;      // dedendum radius (ISO-ish)
    const double rb = rp * std::cos(pa);  // base radius for involute

    // Backlash as small angular trim on each flank at pitch:
    // Convert diametral backlash (mm) to angular reduction around pitch circle.
    const double arcAtPitch = g.backlash / rp;  // radians removed symmetrically

    // Half tooth angle at center:
    const double toothAngle = 2.0 * M_PI / z;
    const double halfTooth = 0.5 * toothAngle;

    // Involute from base to addendum:
    const double t_add = SolveTForRadius(rb, ra);
    const int N = std::max(8, (int)std::ceil(30 * (t_add + 0.1)));  // samples

    std::vector<gp_Pnt> flank;
    flank.reserve(N);
    for (int i = 0; i <= N; ++i) {
        double t = (double)i / (double)N * t_add;
        flank.push_back(InvoluteXY(rb, t));
    }

    // Rotate flanks to center tooth and apply backlash trim near pitch:
    // We construct ONE tooth by:
    //  - left flank rotated by +(halfTooth - delta)
    //  - right flank mirrored & rotated by -(halfTooth - delta)
    const double delta = 0.5 * arcAtPitch;  // small angular trim per flank

    auto rotPoint = [](const gp_Pnt& p, double ang) {
        double c = std::cos(ang), s = std::sin(ang);
        return gp_Pnt(c * p.X() - s * p.Y(), s * p.X() + c * p.Y(), 0);
    };

    std::vector<gp_Pnt> leftFlank, rightFlank;
    leftFlank.reserve(flank.size());
    rightFlank.reserve(flank.size());

    // left = CCW side
    for (auto& p : flank) {
        leftFlank.push_back(rotPoint(p, +(halfTooth - delta)));
    }
    // right = mirror (x, -y) then rotate negative
    for (int i = (int)flank.size() - 1; i >= 0; --i) {
        gp_Pnt q(flank[i].X(), -flank[i].Y(), 0);
        rightFlank.push_back(rotPoint(q, -(halfTooth - delta)));
    }

    // Root arc between the two flanks at radius rf
    // Compute intersection points where involutes cross root circle (if rb < rf)
    auto onCircle = [](double r, double ang) { return gp_Pnt(r * std::cos(ang), r * std::sin(ang), 0); };

    // Angles of flank endpoints at root (approx by looking at first valid > rf)
    auto firstAboveR = [&](const std::vector<gp_Pnt>& pts, bool leftSide) -> int {
        for (int i = 0; i < (int)pts.size(); ++i) {
            double r = std::hypot(pts[i].X(), pts[i].Y());
            if (r > rf + 1e-6) return i;
        }
        return (int)pts.size() - 1;  // fallback
    };

    int iL = firstAboveR(leftFlank, true);
    int iR = firstAboveR(rightFlank, false);

    gp_Pnt pL = leftFlank[iL];
    gp_Pnt pR = rightFlank[iR];

    double angL = std::atan2(pL.Y(), pL.X());
    double angR = std::atan2(pR.Y(), pR.X());

    // Ensure angR < angL for root arc direction (CW from left to right)
    while (angR > angL) angR -= 2 * M_PI;

    // --- Build wire: left flank up to addendum, addendum arc, right flank down, root arc back to left ---

    BRepBuilderAPI_MakeWire w;

    auto addPolyline = [&](const std::vector<gp_Pnt>& pts) {
        for (size_t i = 1; i < pts.size(); ++i) {
            w.Add(BRepBuilderAPI_MakeEdge(pts[i - 1], pts[i]));
        }
    };

    // 1) left flank
    addPolyline(leftFlank);

    // 2) small tip relief (optional), else add addendum arc between left/right at ra
    gp_Pnt tipL = leftFlank.back();
    gp_Pnt tipR = rightFlank.front();

    if (g.tipRelief > 0.0) {
        // Create tiny flat at the top (clip a small chord)
        const double tr = std::min(g.tipRelief, 0.3 * m);  // keep modest
        gp_Pnt dir(std::cos(+halfTooth), std::sin(+halfTooth), 0);
        gp_Pnt tipA = tipL;
        tipA.Translate(gp_Vec(-dir.X() * tr, -dir.Y() * tr, 0));
        gp_Pnt tipB = tipR;
        tipB.Translate(gp_Vec(+dir.X() * tr, +dir.Y() * tr, 0));
        w.Add(BRepBuilderAPI_MakeEdge(tipA, tipB));
    } else {
        // circular arc at addendum
        //
        gp_Circ addCirc(gp_Ax2(gp_Pnt(0, 0, 0), gp::DZ()), ra);
        Handle(Geom_TrimmedCurve) addArc = GC_MakeArcOfCircle(addCirc, tipL, tipR, Standard_True).Value();
        w.Add(BRepBuilderAPI_MakeEdge(addArc));
    }

    // 3) right flank back to root
    addPolyline(rightFlank);

    // 4) root arc from right to left along rf
    Handle(Geom_Circle) rootCircle = new Geom_Circle(gp_Ax2(gp_Pnt(0, 0, 0), gp::DZ()), rf);
    Handle(Geom_TrimmedCurve) rootArc = new Geom_TrimmedCurve(rootCircle, angR, angL, false);
    w.Add(BRepBuilderAPI_MakeEdge(rootArc));

    TopoDS_Wire toothWire = w.Wire();

    TopoDS_Face toothFace = BRepBuilderAPI_MakeFace(toothWire);

    if (g.rootFillet > 0.0) {
        BRepFilletAPI_MakeFillet2d mkf(toothFace);
        for (TopExp_Explorer ex(toothFace, TopAbs_VERTEX); ex.More(); ex.Next()) {
            mkf.AddFillet(TopoDS::Vertex(ex.Current()), g.rootFillet);
        }
        mkf.Build();
        if (mkf.IsDone()) {
            toothFace = TopoDS::Face(mkf.Shape());  // <-- statt direktem '=' auf Face
        }
    }

    return toothFace;
}

/// Rotate a shape around Z by angle (rad)
static TopoDS_Shape RotZ(const TopoDS_Shape& s, double ang) {
    gp_Trsf t;
    t.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp::DZ()), ang);
    return BRepBuilderAPI_Transform(s, t, /*copy*/ true).Shape();
}

ShapePtr SimpleGear::MakeSpur(const GearSpec& g) {
    if (g.teeth < 6 || g.module <= 0.0 || g.thickness <= 0.0) {
        throw std::invalid_argument("SimpleGear::MakeSpur: invalid parameters");
    }

    // 1) build one tooth 2D face
    TopoDS_Face tooth = MakeTooth2D(g);

    // 2) polar array & fuse (2D)
    TopoDS_Shape acc = tooth;
    const double step = 2.0 * M_PI / (double)g.teeth;
    for (int i = 1; i < g.teeth; ++i) {
        TopoDS_Shape ti = RotZ(tooth, step * i);
        acc = BRepAlgoAPI_Fuse(acc, ti).Shape();
    }

    // 3) extrude to thickness
    gp_Vec dir(0, 0, g.thickness);
    TopoDS_Shape solid = BRepPrimAPI_MakePrism(TopoDS::Face(acc), dir, false, true).Shape();

    // 4) drill bore
    if (g.bore > 0.0) {
        TopoDS_Shape bore = BRepPrimAPI_MakeCylinder(0.5 * g.bore, g.thickness * 1.2).Shape();
        // center through thickness: cut
        solid = BRepAlgoAPI_Cut(solid, bore).Shape();
    }

    return std::make_shared<Shape>(solid);
}

}  // namespace mech
