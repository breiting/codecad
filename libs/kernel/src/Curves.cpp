#include <BRepAdaptor_Curve.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRep_Tool.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BezierCurve.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <ccad/base/Exception.hpp>
#include <ccad/base/Shape.hpp>
#include <ccad/construct/Revolve.hpp>
#include <ccad/geom/Curves.hpp>
#include <ccad/geom/Cylinder.hpp>
#include <ccad/ops/Boolean.hpp>

#include "internal/geom/ShapeHelper.hpp"

using namespace ccad::construct;
using namespace ccad::ops;

namespace ccad::geom {

// -------------------------- Interpolate THROUGH points -----------------------

Shape BSplineXZ_Interpolate(const std::vector<Vec2>& pts, int degree, double tol) {
    if (pts.size() < 2) throw Exception("BSplineXZ_Interpolate: need at least 2 points");
    degree = std::clamp(degree, 2, 5);

    TColgp_Array1OfPnt arr(1, static_cast<Standard_Integer>(pts.size()));
    for (Standard_Integer i = 1; i <= arr.Upper(); ++i) {
        const auto& p = pts[static_cast<size_t>(i - 1)];
        arr.SetValue(i, gp_Pnt(p.x, 0.0, p.y));
    }

    GeomAPI_PointsToBSpline api(arr, /*DegMin*/ degree, /*DegMax*/ degree, GeomAbs_C2, tol);
    if (!api.IsDone()) throw Exception("BSplineXZ_Interpolate: OCCT failed");
    Handle(Geom_BSplineCurve) c = api.Curve();

    TopoDS_Edge e = BRepBuilderAPI_MakeEdge(c);
    return WrapOcctShape(e);
}

// -------------------------- From CONTROL points (NURBS) ----------------------

Shape BSplineXZ_Control(const std::vector<Vec2>& ctrl, int degree, bool periodic, const std::vector<double>* weights) {
    if (ctrl.size() < 2) throw Exception("BSplineXZ_Control: need >=2 control points");
    degree = std::clamp(degree, 1, 8);
    const Standard_Integer n = static_cast<Standard_Integer>(ctrl.size());

    // Control points
    TColgp_Array1OfPnt poles(1, n);
    for (Standard_Integer i = 1; i <= n; ++i) {
        const auto& p = ctrl[static_cast<size_t>(i - 1)];
        poles.SetValue(i, gp_Pnt(p.x, 0.0, p.y));
    }

    // Weights (optional → rational)
    Handle(TColStd_HArray1OfReal) wts;
    if (weights && !weights->empty()) {
        if (weights->size() != ctrl.size()) throw Exception("BSplineXZ_Control: weights size mismatch");
        wts = new TColStd_HArray1OfReal(1, n);
        for (Standard_Integer i = 1; i <= n; ++i) wts->SetValue(i, (*weights)[static_cast<size_t>(i - 1)]);
    }

    // Uniform knots & multiplicities
    // number of knots k = n - degree + (periodic ? 1 : 0)
    // For non-periodic (open) curve, use open knot vector with end mult = degree+1
    Standard_Integer nbKnots;
    if (periodic) {
        nbKnots = n;  // periodic: simple uniform
    } else {
        nbKnots = n - degree + 1;
        if (nbKnots < 2) throw Exception("BSplineXZ_Control: invalid degree vs points");
    }

    TColStd_Array1OfReal knots(1, nbKnots);
    TColStd_Array1OfInteger mults(1, nbKnots);

    if (periodic) {
        for (Standard_Integer i = 1; i <= nbKnots; ++i) {
            knots.SetValue(i, static_cast<Standard_Real>(i - 1));
            mults.SetValue(i, 1);
        }
    } else {
        // Open uniform [0..m] with end multiplicity = degree+1
        const Standard_Real m = static_cast<Standard_Real>(nbKnots - 1);
        for (Standard_Integer i = 1; i <= nbKnots; ++i) {
            knots.SetValue(i, (m == 0.0) ? 0.0 : (static_cast<Standard_Real>(i - 1) / m));
            if (i == 1 || i == nbKnots)
                mults.SetValue(i, degree + 1);
            else
                mults.SetValue(i, 1);
        }
    }

    Handle(Geom_BSplineCurve) c = new Geom_BSplineCurve(poles, knots, mults, degree, periodic);

    TopoDS_Edge e = BRepBuilderAPI_MakeEdge(c);
    return WrapOcctShape(e);
}

// --------------------------------- Bézier ------------------------------------

Shape BezierXZ(const std::vector<Vec2>& ctrl, const std::vector<double>* weights) {
    if (ctrl.size() < 2) throw Exception("BezierXZ: need >=2 control points");
    const Standard_Integer n = static_cast<Standard_Integer>(ctrl.size());

    TColgp_Array1OfPnt poles(1, n);
    for (Standard_Integer i = 1; i <= n; ++i) {
        const auto& p = ctrl[static_cast<size_t>(i - 1)];
        poles.SetValue(i, gp_Pnt(p.x, 0.0, p.y));
    }

    Handle(Geom_BezierCurve) c;
    if (weights && !weights->empty()) {
        if (weights->size() != ctrl.size()) throw Exception("BezierXZ: weights size mismatch");
        TColStd_Array1OfReal w(1, n);
        for (Standard_Integer i = 1; i <= n; ++i) w.SetValue(i, (*weights)[static_cast<size_t>(i - 1)]);
        c = new Geom_BezierCurve(poles, w);
    } else {
        c = new Geom_BezierCurve(poles);
    }

    TopoDS_Edge e = BRepBuilderAPI_MakeEdge(c);
    return WrapOcctShape(e);
}

// ------------------------------- Close & Face --------------------------------

Shape CloseCurve(const Shape& edge) {
    auto oe = ShapeAsOcct(edge);
    if (!oe || oe->Occt().ShapeType() != TopAbs_EDGE) throw Exception("CurveClose: expected a single edge");

    TopoDS_Edge E = TopoDS::Edge(oe->Occt());
    // build wire (edge + closing segment)
    BRepBuilderAPI_MakeWire mw;
    mw.Add(E);

    // get end points
    BRepAdaptor_Curve c(E);
    gp_Pnt p0 = c.Value(c.FirstParameter());
    gp_Pnt p1 = c.Value(c.LastParameter());

    if (!p0.IsEqual(p1, 1e-9)) {
        mw.Add(BRepBuilderAPI_MakeEdge(p1, p0));
    }
    if (!mw.IsDone()) throw Exception("CurveClose: wire failed");
    return WrapOcctShape(mw.Wire());
}

Shape CloseCurveZ(const Shape& edge) {
    auto os = ShapeAsOcct(edge);
    if (!os) throw std::runtime_error("CloseCurveZ: non-OCCT shape implementation");

    // Extract the edge as wire
    TopoDS_Shape s = os->Occt();
    TopoDS_Wire w;
    if (s.ShapeType() == TopAbs_EDGE) {
        w = BRepBuilderAPI_MakeWire(TopoDS::Edge(s));
    } else if (s.ShapeType() == TopAbs_WIRE) {
        w = TopoDS::Wire(s);
    } else {
        throw std::runtime_error("CloseCurveZ: expected edge or wire");
    }

    // Get curve geometry for endpoints
    TopoDS_Edge e = TopoDS::Edge(TopoDS_Iterator(w).Value());
    BRepAdaptor_Curve c(e);

    gp_Pnt pFirst = BRep_Tool::Pnt(TopExp::FirstVertex(e));
    gp_Pnt pLast = BRep_Tool::Pnt(TopExp::LastVertex(e));

    // Project to Z-axis (x=0, y=0) while keeping z
    gp_Pnt pFirstZ(0.0, 0.0, pFirst.Z());
    gp_Pnt pLastZ(0.0, 0.0, pLast.Z());

    // Build two closing edges
    TopoDS_Edge eClose1 = BRepBuilderAPI_MakeEdge(pFirst, pFirstZ);
    TopoDS_Edge eClose2 = BRepBuilderAPI_MakeEdge(pLastZ, pLast);

    // Axis edge along Z
    TopoDS_Edge eAxis = BRepBuilderAPI_MakeEdge(pFirstZ, pLastZ);

    // Make wire: curve + closing edges
    BRepBuilderAPI_MakeWire mw;
    mw.Add(w);
    mw.Add(eClose1);
    mw.Add(eAxis);
    mw.Add(eClose2);

    if (!mw.IsDone()) throw std::runtime_error("CloseCurveZ: failed to build closed wire");

    // Face from wire
    TopoDS_Face f = BRepBuilderAPI_MakeFace(mw.Wire());
    return WrapOcctShape(f);
}

Shape Lathe(const LatheSpec& spec) {
    if (spec.points.size() < 2) {
        throw std::invalid_argument("Lathe: at least 2 points required");
    }

    Shape edge;
    edge = BezierXZ(spec.points);

    Shape face = CloseCurveZ(edge);
    auto outer = RevolveZ(face, spec.angleDeg);

    if (spec.thickness <= 0.0) {
        return outer;
    }

    // In case of a wall thickness, build inner and subtract
    Shape innerEdge;
    std::vector<Vec2> innerPts;
    innerPts.reserve(spec.points.size());
    for (auto& p : spec.points) {
        innerPts.emplace_back(Vec2{p.x - spec.thickness, p.y});
    }

    innerEdge = BezierXZ(innerPts);
    auto inner = RevolveZ(CloseCurveZ(innerEdge), spec.angleDeg);
    auto wall = Difference(outer, inner);

    return wall;
}

}  // namespace ccad::geom
