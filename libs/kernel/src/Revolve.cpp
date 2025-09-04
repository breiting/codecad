#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include "ccad/base/Math.hpp"
#include "internal/geom/ShapeHelper.hpp"

namespace ccad::construct {

Shape RevolveZ(const Shape& shp, double angleDeg) {
    auto s = ShapeAsOcct(shp);
    if (!s) throw std::runtime_error("Revolve: non-OCCT shape implementation");

    const double angle = DegToRad(angleDeg);
    gp_Ax1 axis(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));

    TopAbs_ShapeEnum shapeType = s->Occt().ShapeType();
    TopoDS_Shape out;
    if (shapeType == TopAbs_FACE) {
        out = BRepPrimAPI_MakeRevol(TopoDS::Face(s->Occt()), axis, angle).Shape();
    } else if (shapeType == TopAbs_WIRE) {
        out = BRepPrimAPI_MakeRevol(TopoDS::Wire(s->Occt()), axis, angle).Shape();
    } else {
        // Best-effort
        out = BRepPrimAPI_MakeRevol(s->Occt(), axis, angle).Shape();
    }
    return WrapOcctShape(out);
}
}  // namespace ccad::construct
