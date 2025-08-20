#include "geometry/Extrude.hpp"

#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

namespace geometry {

ShapePtr ExtrudeZ(const ShapePtr& face, double height) {
    if (!face || height == 0.0) return face;
    if (face->Get().ShapeType() != TopAbs_FACE) return face;
    gp_Vec dz(0, 0, height);
    TopoDS_Shape s = BRepPrimAPI_MakePrism(TopoDS::Face(face->Get()), dz).Shape();
    return std::make_shared<Shape>(s);
}

ShapePtr RevolveZ(const ShapePtr& shp, double angle_deg) {
    if (!shp || angle_deg == 0.0) return shp;
    const double ang = angle_deg * M_PI / 180.0;
    gp_Ax1 axis(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
    TopAbs_ShapeEnum tp = shp->Get().ShapeType();
    TopoDS_Shape out;
    if (tp == TopAbs_FACE) {
        out = BRepPrimAPI_MakeRevol(TopoDS::Face(shp->Get()), axis, ang).Shape();
    } else if (tp == TopAbs_WIRE) {
        out = BRepPrimAPI_MakeRevol(TopoDS::Wire(shp->Get()), axis, ang).Shape();
    } else {
        // Best-effort
        out = BRepPrimAPI_MakeRevol(shp->Get(), axis, ang).Shape();
    }
    return std::make_shared<Shape>(out);
}

}  // namespace geometry
