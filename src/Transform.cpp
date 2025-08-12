#include "geo/Transform.hpp"

#include <BRepBuilderAPI_Transform.hxx>
#include <gp_Ax1.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>

namespace geo {

static constexpr double kPi = 3.14159265358979323846;
static double DegToRad(double d) {
    return d * kPi / 180.0;
}

ShapePtr Translate(const ShapePtr& s, double dx, double dy, double dz) {
    if (!s) return nullptr;
    gp_Trsf tr;
    tr.SetTranslation(gp_Vec(dx, dy, dz));
    TopoDS_Shape out = BRepBuilderAPI_Transform(s->Get(), tr, true).Shape();
    return std::make_shared<Shape>(out);
}

ShapePtr RotateX(const ShapePtr& s, double degrees) {
    if (!s) return nullptr;
    gp_Trsf tr;
    tr.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0)), DegToRad(degrees));
    TopoDS_Shape out = BRepBuilderAPI_Transform(s->Get(), tr, true).Shape();
    return std::make_shared<Shape>(out);
}

ShapePtr RotateY(const ShapePtr& s, double degrees) {
    if (!s) return nullptr;
    gp_Trsf tr;
    tr.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0)), DegToRad(degrees));
    TopoDS_Shape out = BRepBuilderAPI_Transform(s->Get(), tr, true).Shape();
    return std::make_shared<Shape>(out);
}

ShapePtr RotateZ(const ShapePtr& s, double degrees) {
    if (!s) return nullptr;
    gp_Trsf tr;
    tr.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), DegToRad(degrees));
    TopoDS_Shape out = BRepBuilderAPI_Transform(s->Get(), tr, true).Shape();
    return std::make_shared<Shape>(out);
}

ShapePtr Scale(const ShapePtr& s, double factor) {
    if (!s) return nullptr;
    gp_Trsf tr;
    tr.SetScale(gp_Pnt(0, 0, 0), factor);
    TopoDS_Shape out = BRepBuilderAPI_Transform(s->Get(), tr, true).Shape();
    return std::make_shared<Shape>(out);
}

}  // namespace geo
