#include "geometry/Primitives.hpp"

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeWedge.hxx>
#include <cmath>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

// NEW INCLUDES
//
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

// ----------------------------------
// TODO: OLD CODE
namespace geometry {

ShapePtr MakeBox(double x, double y, double z) {
    TopoDS_Shape s = BRepPrimAPI_MakeBox(x, y, z).Shape();
    return std::make_shared<Shape>(s);
}

ShapePtr MakeCylinder(double diameter, double height) {
    TopoDS_Shape s = BRepPrimAPI_MakeCylinder(0.5 * diameter, height).Shape();
    return std::make_shared<Shape>(s);
}

ShapePtr MakeCone(double diameter1, double diameter2, double height) {
    TopoDS_Shape s = BRepPrimAPI_MakeCone(diameter1 * 0.5, diameter2 * 0.5, height).Shape();
    return std::make_shared<Shape>(s);
}

ShapePtr MakeWedge(double dx, double dy, double dz, double ltx) {
    TopoDS_Shape s = BRepPrimAPI_MakeWedge(dx, dy, dz, ltx).Shape();
    return std::make_shared<Shape>(s);
}

ShapePtr MakeSphere(double diameter) {
    TopoDS_Shape s = BRepPrimAPI_MakeSphere(0.5 * diameter).Shape();
    return std::make_shared<Shape>(s);
}

static TopoDS_Face MakeRegularPolygonFace(int n, double acrossFlats) {
    // For Hex: across_flats = 2*Ri, Ri = Rc*cos(30°), also Rc = A/√3
    const double Rc = acrossFlats / std::sqrt(3.0);
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

}  // namespace geometry
