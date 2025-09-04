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

#include "ccad/base/Exception.hpp"
#include "ccad/base/Status.hpp"
#include "private/geom/ShapeHelper.hpp"

namespace ccad {
namespace geom {
Shape Box(double sx, double sy, double sz) {
    if (sx <= 0 || sy <= 0 || sz <= 0) throw Exception("Box: sizes must be > 0", Status::ERROR_OCCT);
    TopoDS_Shape s = BRepPrimAPI_MakeBox(sx, sy, sz).Shape();
    return WrapOcctShape(s);
}

Shape Cylinder(double diameter, double height) {
    if (diameter <= 0 || height <= 0) throw Exception("Cylinder: d,h must be > 0", Status::ERROR_OCCT);
    gp_Ax2 ax(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));  // Z-up
    TopoDS_Shape s = BRepPrimAPI_MakeCylinder(ax, diameter * 0.5, height).Shape();
    return WrapOcctShape(s);
}

Shape Cone(double diameter1, double diameter2, double height) {
    if (diameter1 <= 0 || diameter2 <= 0 || height <= 0)
        throw Exception("Cone: d1,d2,h must be > 0", Status::ERROR_OCCT);
    gp_Ax2 ax(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));  // Z-up
    TopoDS_Shape s = BRepPrimAPI_MakeCone(ax, diameter1 * 0.5, diameter2 * 0.5, height).Shape();
    return WrapOcctShape(s);
}

Shape Wedge(double dx, double dy, double dz, double ltx) {
    if (dx <= 0 || dy <= 0 || dz <= 0 || ltx <= 0)
        throw Exception("Wedge: dx,dy,dz,ltx must be > 0", Status::ERROR_OCCT);
    TopoDS_Shape s = BRepPrimAPI_MakeWedge(dx, dy, dz, ltx).Shape();
    return WrapOcctShape(s);
}

Shape Sphere(double diameter) {
    if (diameter <= 0) throw Exception("Sphere: diameter must be > 0", Status::ERROR_OCCT);
    TopoDS_Shape s = BRepPrimAPI_MakeSphere(0.5 * diameter).Shape();
    return WrapOcctShape(s);
}

// internal helper function
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

Shape HexPrism(double across_flats, double height) {
    if (across_flats <= 0 || height <= 0)
        throw Exception("HexPrism: across_flats,height must be > 0", Status::ERROR_OCCT);
    TopoDS_Face face = MakeRegularPolygonFace(6, across_flats);
    TopoDS_Shape s = BRepPrimAPI_MakePrism(face, gp_Vec(0, 0, height)).Shape();
    return WrapOcctShape(s);
}
}  // namespace geom
}  // namespace ccad
