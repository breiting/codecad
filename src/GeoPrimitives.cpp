#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>

#include "Geo/Primitives.hpp"

namespace Geo {

ShapePtr MakeBox(double x, double y, double z) {
    TopoDS_Shape s = BRepPrimAPI_MakeBox(x, y, z).Shape();
    return std::make_shared<Shape>(s);
}

ShapePtr MakeCylinder(double r, double h) {
    TopoDS_Shape s = BRepPrimAPI_MakeCylinder(r, h).Shape();
    return std::make_shared<Shape>(s);
}

}  // namespace Geo
