#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <cmath>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include "Geo/Primitives.hpp"

namespace Geo {

ShapePtr MakeBox(double x, double y, double z) {
    TopoDS_Shape s = BRepPrimAPI_MakeBox(x, y, z).Shape();
    return std::make_shared<Shape>(s);
}

ShapePtr MakeCylinder(double diameter, double height) {
    const double r = 0.5 * diameter;
    TopoDS_Shape s = BRepPrimAPI_MakeCylinder(r, height).Shape();
    return std::make_shared<Shape>(s);
}

static TopoDS_Face MakeRegularPolygonFace(int n, double across_flats) {
    // Für Hex: across_flats = 2*Ri, Ri = Rc*cos(30°), also Rc = A/√3
    const double Rc = across_flats / std::sqrt(3.0);
    BRepBuilderAPI_MakePolygon poly;
    for (int i = 0; i < n; ++i) {
        double ang = (2.0 * M_PI * i) / n;
        poly.Add(gp_Pnt(Rc * std::cos(ang), Rc * std::sin(ang), 0.0));
    }
    poly.Close();
    TopoDS_Wire w = poly.Wire();
    return BRepBuilderAPI_MakeFace(w).Face();
}

ShapePtr MakeHexPrism(double across_flats, double height) {
    TopoDS_Face face = MakeRegularPolygonFace(6, across_flats);
    TopoDS_Shape s = BRepPrimAPI_MakePrism(face, gp_Vec(0, 0, height)).Shape();
    return std::make_shared<Shape>(s);
}

}  // namespace Geo
