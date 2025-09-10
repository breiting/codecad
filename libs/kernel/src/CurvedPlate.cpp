#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFill.hxx>
#include <BRepFill_Filling.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <GeomAPI_PointsToBSplineSurface.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Precision.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Wire.hxx>
#include <ccad/geom/CurvedPlate.hpp>

#include "internal/geom/ShapeHelper.hpp"

using namespace ccad;

namespace {

// height field z(x,y) from two directions u,v
static double heightAt(double x, double y, const glm::dvec2& U, const glm::dvec2& V, double kU, double kV,
                       geom::BendLaw law) {
    const double u = U.x * x + U.y * y;  // projection to U
    const double v = V.x * x + V.y * y;  // projection to V
    switch (law) {
        case geom::BendLaw::Paraboloid:
            // z = 0.5*kU*u^2 + 0.5*kV*v^2
            return 0.5 * kU * u * u + 0.5 * kV * v * v;
        case geom::BendLaw::Sine:
            // z = (1/kU) sin(kU u) + (1/kV) sin(kV v)   (safe for k≈0)
            return (kU != 0.0 ? std::sin(kU * u) / kU : 0.0) + (kV != 0.0 ? std::sin(kV * v) / kV : 0.0);
    }
    return 0.0;
}

// Build BSpline surface from an XY grid with given height function
static Handle(Geom_BSplineSurface)
    makeBSplineSurface(double sx, double sy, int nu, int nv, const std::function<double(double, double)>& zfun) {
    const int iu0 = 1, iu1 = std::max(2, nu);
    const int iv0 = 1, iv1 = std::max(2, nv);
    TColgp_Array2OfPnt grid(iu0, iu1, iv0, iv1);

    // center the plate at origin for convenience
    for (int i = iu0; i <= iu1; ++i) {
        double tx = (double)(i - iu0) / (double)(iu1 - iu0);
        double x = (tx - 0.5) * sx;
        for (int j = iv0; j <= iv1; ++j) {
            double ty = (double)(j - iv0) / (double)(iv1 - iv0);
            double y = (ty - 0.5) * sy;
            double z = zfun(x, y);
            grid.SetValue(i, j, gp_Pnt(x, y, z));
        }
    }

    GeomAPI_PointsToBSplineSurface builder(grid, 3, 3, GeomAbs_C2, 1.0e-6);
    if (!builder.IsDone()) throw std::runtime_error("CurvedPlate: BSpline surface build failed");
    return builder.Surface();
}

static TopoDS_Face makeFaceFromSurface(const Handle(Geom_Surface) & s, double /*sx*/, double /*sy*/) {
    // Build a rectangular face trimmed in U,V by projecting XY extents to UV space.
    // With builder.Surface() default UV parametrization, BRepBuilderAPI_MakeFace(surface, tol) works.
    return BRepBuilderAPI_MakeFace(s, 1.0e-6).Face();
}

// Extract outer wire of a face (assuming one, typical after MakeFace)
static TopoDS_Wire OuterWire(const TopoDS_Face& f) {
    return BRepTools::OuterWire(f);
}

// Make ruled side face between two matching edges
static TopoDS_Face RuledFace(const TopoDS_Edge& eTop, const TopoDS_Edge& eBot) {
    BRepOffsetAPI_ThruSections mk(/*isSolid*/ false, /*isRuled*/ true, /*prec*/ Precision::Confusion());
    mk.AddWire(BRepBuilderAPI_MakeWire(eTop));
    mk.AddWire(BRepBuilderAPI_MakeWire(eBot));
    mk.Build();
    if (!mk.IsDone()) throw std::runtime_error("RuledFace: ThruSections failed");
    return TopoDS::Face(mk.Shape());
}
static TopoDS_Shape SewToSolid(const std::vector<TopoDS_Face>& faces) {
    BRepBuilderAPI_Sewing sew(1.0e-6);
    for (const auto& f : faces) sew.Add(f);
    sew.Perform();
    TopoDS_Shape shell = sew.SewedShape();
    if (shell.IsNull()) throw std::runtime_error("CurvedPlate: sewing failed");
    if (shell.ShapeType() == TopAbs_SHELL) {
        TopoDS_Solid solid = BRepBuilderAPI_MakeSolid(TopoDS::Shell(shell));
        return TopoDS_Shape(solid);  // explicit upcast
    }
    // If already a solid or something else, just return
    return shell;
}

}  // namespace

namespace ccad::geom {

Shape CurvedPlate(const CurvedPlateSpec& inSpec) {
    auto spec = inSpec;

    // Normalize in-plane directions
    auto safeNorm = [](glm::dvec2 v) -> glm::dvec2 {
        double n = glm::length(v);
        return (n > 1e-12) ? (v / n) : glm::dvec2(1.0, 0.0);
    };
    glm::dvec2 U = safeNorm(spec.dirU);
    glm::dvec2 V = safeNorm(spec.dirV);

    // z(x,y) sampler
    auto zfun = [&](double x, double y) { return heightAt(x, y, U, V, spec.kU, spec.kV, spec.law); };

    // 1) Mid-surface as BSpline
    Handle(Geom_BSplineSurface) sMid = makeBSplineSurface(spec.sizeX, spec.sizeY, spec.nu, spec.nv, zfun);
    TopoDS_Face fMid = makeFaceFromSurface(sMid, spec.sizeX, spec.sizeY);

    // 2) Offset surfaces (top/bottom) by +/- t/2 along the surface normal
    const double halfT = 0.5 * std::max(1e-6, spec.thickness);
    Handle(Geom_OffsetSurface) sTop(new Geom_OffsetSurface(sMid, +halfT));
    Handle(Geom_OffsetSurface) sBot(new Geom_OffsetSurface(sMid, -halfT));
    TopoDS_Face fTop = makeFaceFromSurface(sTop, spec.sizeX, spec.sizeY);
    TopoDS_Face fBot = makeFaceFromSurface(sBot, spec.sizeX, spec.sizeY);

    // 3) Side walls: we need the 4 boundary edges of top/bottom, in matching order.
    //    Extract outer wire, then iterate edges in the same sequence.
    TopoDS_Wire wTop = OuterWire(fTop);
    TopoDS_Wire wBot = OuterWire(fBot);

    std::vector<TopoDS_Edge> edgesTop, edgesBot;
    for (TopExp_Explorer ex(wTop, TopAbs_EDGE); ex.More(); ex.Next()) edgesTop.push_back(TopoDS::Edge(ex.Current()));
    for (TopExp_Explorer ex(wBot, TopAbs_EDGE); ex.More(); ex.Next()) edgesBot.push_back(TopoDS::Edge(ex.Current()));

    // Ensure we have 4 edges (rect trim). If not, we still try to pair by count.
    const size_t n = std::min(edgesTop.size(), edgesBot.size());
    if (n < 3) throw std::runtime_error("CurvedPlate: unexpected boundary edge count");

    // Build ruled side faces
    std::vector<TopoDS_Face> faces;
    faces.reserve((size_t)n + 2);
    faces.push_back(fTop);
    faces.push_back(fBot);
    for (size_t i = 0; i < n; ++i) {
        faces.push_back(RuledFace(edgesTop[i], edgesBot[i]));
    }

    // 4) Sew and make solid
    TopoDS_Shape solid = SewToSolid(faces);
    return WrapOcctShape(solid);
}

}  // namespace ccad::geom
