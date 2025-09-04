#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>

#include "ccad/ops/Boolean.hpp"
#include "ccad/ops/Transform.hpp"
#include "private/geom/ShapeHelper.hpp"

namespace ccad {
namespace ops {

Shape Union(const Shape& a, const Shape& b) {
    auto oa = ShapeAsOcct(a), ob = ShapeAsOcct(b);
    if (!oa || !ob) throw std::runtime_error("Union: non-OCCT shape implementation");
    BRepAlgoAPI_Fuse algo(oa->Occt(), ob->Occt());
    algo.SetRunParallel(true);
    algo.Build();
    if (!algo.IsDone()) throw std::runtime_error("Union failed");
    return WrapOcctShape(algo.Shape());
}

Shape Difference(const Shape& a, const Shape& b) {
    auto oa = ShapeAsOcct(a), ob = ShapeAsOcct(b);
    if (!oa || !ob) throw std::runtime_error("Difference: non-OCCT shape implementation");
    BRepAlgoAPI_Cut algo(oa->Occt(), ob->Occt());
    algo.SetRunParallel(true);
    algo.Build();
    if (!algo.IsDone()) throw std::runtime_error("Difference failed");
    return WrapOcctShape(algo.Shape());
}

Shape Intersection(const Shape& a, const Shape& b) {
    auto oa = ShapeAsOcct(a), ob = ShapeAsOcct(b);
    if (!oa || !ob) throw std::runtime_error("Intersection: non-OCCT shape implementation");
    BRepAlgoAPI_Common algo(oa->Occt(), ob->Occt());
    algo.SetRunParallel(true);
    algo.Build();
    if (!algo.IsDone()) throw std::runtime_error("Intersection failed");
    return WrapOcctShape(algo.Shape());
}

// --- Transforms -------------------------------------------------------------

static Shape apply_trsf(const Shape& s, const gp_Trsf& tr) {
    auto os = ShapeAsOcct(s);
    if (!os) throw std::runtime_error("Transform: non-OCCT shape implementation");
    BRepBuilderAPI_Transform t(os->Occt(), tr, /*copy*/ true);
    t.Build();
    if (!t.IsDone()) throw std::runtime_error("Transform failed");
    return WrapOcctShape(t.Shape());
}

Shape Translate(const Shape& s, double dx, double dy, double dz) {
    gp_Trsf tr;
    tr.SetTranslation(gp_Vec(dx, dy, dz));
    return apply_trsf(s, tr);
}

Shape RotateX(const Shape& s, double deg) {
    gp_Trsf tr;
    tr.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0)), deg * M_PI / 180.0);
    return apply_trsf(s, tr);
}
Shape RotateY(const Shape& s, double deg) {
    gp_Trsf tr;
    tr.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0)), deg * M_PI / 180.0);
    return apply_trsf(s, tr);
}
Shape RotateZ(const Shape& s, double deg) {
    gp_Trsf tr;
    tr.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), deg * M_PI / 180.0);
    return apply_trsf(s, tr);
}

Shape ScaleUniform(const Shape& s, double factor) {
    gp_Trsf tr;
    tr.SetScale(gp_Pnt(0, 0, 0), factor);
    return apply_trsf(s, tr);
}
}  // namespace ops
}  // namespace ccad
