#include "ccad/geom/Primitives.hpp"

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeWedge.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include "ccad/common/Exception.hpp"
#include "ccad/common/Status.hpp"
#include "ccad/geom/Primitives.hpp"
#include "private/geometry/OcctShape.hpp"

namespace ccad {
Shape Box(double sx, double sy, double sz) {
    if (sx <= 0 || sy <= 0 || sz <= 0) throw Exception("Box: sizes must be > 0", Status::ERROR_OCCT);
    TopoDS_Shape s = BRepPrimAPI_MakeBox(sx, sy, sz).Shape();
    return Shape{std::make_unique<OcctShape>(std::move(s))};
}

Shape Cylinder(double diameter, double height) {
    if (diameter <= 0 || height <= 0) throw Exception("Cylinder: d,h must be > 0", Status::ERROR_OCCT);
    const double r = 0.5 * diameter;
    gp_Ax2 ax(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));  // Z-up
    TopoDS_Shape s = BRepPrimAPI_MakeCylinder(ax, r, height).Shape();
    return Shape{std::make_unique<OcctShape>(std::move(s))};
}
}  // namespace ccad
