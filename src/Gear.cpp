#include "geo/Gear.hpp"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <cmath>
#include <gp_Circ.hxx>
#include <gp_Pnt.hxx>
#include <vector>

namespace {

struct GearGeom {
    double m, alpha, rb, rp, ra, rf;  // Modul, Druckwinkel(rad), Basis-, Teil-, Kopf-, Fußkreis
};

static GearGeom compute(int z, double m, double pressure_deg) {
    GearGeom g{};
    g.m = m;
    g.alpha = pressure_deg * M_PI / 180.0;
    g.rp = 0.5 * z * m;
    g.ra = g.rp + m;         // addendum
    g.rf = g.rp - 1.25 * m;  // dedendum
    if (g.rf < 0.1 * m) g.rf = 0.1 * m;
    g.rb = g.rp * std::cos(g.alpha);
    return g;
}

// Involute-Parameterisierung von Basisradius rb bis Kopfkreis ra
static std::vector<gp_Pnt> involute_points(double rb, double ra, int samples, double flip) {
    std::vector<gp_Pnt> pts;
    pts.reserve(samples);
    double tmax = std::sqrt((ra * ra - rb * rb) / (rb * rb));  // approximate
    for (int i = 0; i <= samples; ++i) {
        double t = tmax * (double(i) / samples);
        double x = rb * (std::cos(t) + t * std::sin(t));
        double y = rb * (std::sin(t) - t * std::cos(t));
        pts.emplace_back(flip * x, flip * y, 0.0);
    }
    return pts;
}
static Handle(Geom_BSplineCurve) spline_from(const std::vector<gp_Pnt>& pts) {
    const Standard_Integer n = static_cast<Standard_Integer>(pts.size());
    Handle(TColgp_HArray1OfPnt) arr = new TColgp_HArray1OfPnt(1, n);
    for (Standard_Integer i = 1; i <= n; ++i) {
        arr->SetValue(i, pts[static_cast<size_t>(i - 1)]);
    }
    // 3-Argument-Konstruktor: Punkte, PeriodicFlag, Tolerance
    GeomAPI_Interpolate itp(arr, Standard_False, 1.0e-6);
    itp.Perform();
    return itp.Curve();
}

}  // namespace

namespace geo {

ShapePtr MakeInvoluteGear(int z, double m, double th, double bore, double pressure_deg) {
    if (z < 6) z = 6;
    if (m <= 0) m = 1.0;
    if (th <= 0) th = m;
    if (bore < 0) bore = 0.0;

    GearGeom gg = compute(z, m, pressure_deg);

    // Ein Zahn in 2D (XY) konstruieren: linke & rechte Involute, unten am Fußkreis verbinden,
    // oben am Kopfkreis mit kleinem Kreisbogen zusammenführen.
    const double tooth_angle = 2.0 * M_PI / z;
    const double half_thick = M_PI * m / (2.0);  // auf Teilkreis
    const double rot_half = half_thick / gg.rp;  // Winkel-Versatz

    // Punkte der Involutenflanken
    auto right_pts = involute_points(gg.rb, gg.ra, 12, +1.0);
    auto left_pts = involute_points(gg.rb, gg.ra, 12, -1.0);

    // beide um +/- rot_half um den Ursprung drehen (Teildicke)
    auto rot = [&](gp_Pnt& p, double ang) {
        double c = std::cos(ang), s = std::sin(ang);
        double x = p.X(), y = p.Y();
        p.SetX(c * x - s * y);
        p.SetY(s * x + c * y);
    };
    for (auto& p : right_pts) rot(p, +rot_half);
    for (auto& p : left_pts) rot(p, -rot_half);

    // Drähte bauen: Fußkreisbogen zwischen den unteren Punkten, Kopfkreisbogen oben
    gp_Pnt RbR = right_pts.front();
    gp_Pnt RbL = left_pts.front();
    gp_Pnt RaR = right_pts.back();
    gp_Pnt RaL = left_pts.back();

    BRepBuilderAPI_MakeWire mw;
    // --- Unterer Bogen auf dem Basiskreis (rb) zwischen den Involuten-Startpunkten
    {
        auto ang = [](const gp_Pnt& p) { return std::atan2(p.Y(), p.X()); };
        double aL = ang(RbL);
        double aR = ang(RbR);
        // Sicherheitsfall: Reihenfolge normalisieren
        if (aR < aL) std::swap(aR, aL);
        gp_Circ circ_rb(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), gg.rb);
        GC_MakeArcOfCircle mk(circ_rb, aL, aR, Standard_True);
        mw.Add(BRepBuilderAPI_MakeEdge(mk.Value()));
    }

    // rechte Involute rauf
    {
        Handle(Geom_BSplineCurve) cr = spline_from(right_pts);
        mw.Add(BRepBuilderAPI_MakeEdge(cr));
    }
    // Kopfkreisbogen (ra) von rechts nach links (gegen Uhrzeiger)
    {
        auto ang = [](const gp_Pnt& p) { return std::atan2(p.Y(), p.X()); };
        const double aR = ang(RaR);
        const double aL = ang(RaL);
        gp_Circ circ(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), gg.ra);
        GC_MakeArcOfCircle mk(circ, aR, aL, Standard_True);
        mw.Add(BRepBuilderAPI_MakeEdge(mk.Value()));
    }

    // linke Involute runter
    {
        Handle(Geom_BSplineCurve) cl = spline_from(left_pts);
        mw.Add(BRepBuilderAPI_MakeEdge(cl));
    }

    TopoDS_Wire tooth_wire = mw.Wire();
    TopoDS_Face tooth_face = BRepBuilderAPI_MakeFace(tooth_wire).Face();

    // Zahn extrudieren (dünne Scheibe)
    TopoDS_Shape tooth_solid = BRepPrimAPI_MakePrism(tooth_face, gp_Vec(0, 0, th)).Shape();

    // Polar Array und Fuse
    TopoDS_Shape acc = tooth_solid;
    for (int i = 1; i < z; ++i) {
        gp_Trsf tr;
        tr.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), i * tooth_angle);
        TopoDS_Shape inst = BRepBuilderAPI_Transform(tooth_solid, tr, true).Shape();
        acc = BRepAlgoAPI_Fuse(acc, inst);
    }

    // Fußzylinder abdecken (rf)
    TopoDS_Shape root_disc = BRepPrimAPI_MakeCylinder(gg.rf, th).Shape();
    TopoDS_Shape gear = BRepAlgoAPI_Fuse(acc, root_disc);

    // Bohrung
    if (bore > 0.0) {
        TopoDS_Shape hole = BRepPrimAPI_MakeCylinder(0.5 * bore, th + 0.2).Shape();
        gp_Trsf tz;
        tz.SetTranslation(gp_Vec(0, 0, -0.1));
        hole = BRepBuilderAPI_Transform(hole, tz, true).Shape();
        gear = BRepAlgoAPI_Cut(gear, hole);
    }

    return std::make_shared<Shape>(gear);
}

}  // namespace geo
