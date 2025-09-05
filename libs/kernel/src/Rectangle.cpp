#include <ccad/sketch/Rectangle.hpp>

// OCCT
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopoDS_Face.hxx>
#include <gp_Pnt.hxx>

#include "internal/geom/ShapeHelper.hpp"

namespace ccad::sketch {

Shape Rectangle(double width, double height) {
    if (width <= 0.0 || height <= 0.0) {
        throw std::invalid_argument("Rectangle: width and height must be > 0");
    }

    const double hx = 0.5 * width;
    const double hy = 0.5 * height;

    // Define 4 corners in XY plane (Z = 0)
    gp_Pnt p1(-hx, -hy, 0);
    gp_Pnt p2(+hx, -hy, 0);
    gp_Pnt p3(+hx, +hy, 0);
    gp_Pnt p4(-hx, +hy, 0);

    // Build edges
    TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(p1, p2);
    TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(p2, p3);
    TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge(p3, p4);
    TopoDS_Edge e4 = BRepBuilderAPI_MakeEdge(p4, p1);

    // Make closed wire
    BRepBuilderAPI_MakeWire wireMaker;
    wireMaker.Add(e1);
    wireMaker.Add(e2);
    wireMaker.Add(e3);
    wireMaker.Add(e4);

    if (!wireMaker.IsDone()) {
        throw std::runtime_error("Rectangle: Failed to build wire");
    }

    // Make face from wire
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wireMaker.Wire());

    return WrapOcctShape(face);
}

}  // namespace ccad::sketch
