#include "ccad/construct/Extrude.hpp"

#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include "ccad/base/Exception.hpp"
#include "ccad/base/Status.hpp"
#include "internal/geom/ShapeHelper.hpp"

namespace ccad {
namespace construct {

Shape ExtrudeZ(const Shape& face, double height) {
    if (height == 0.0) throw Exception("Height must be > 0", Status::ERROR_OCCT);
    auto fo = ShapeAsOcct(face);

    if (!fo) throw std::runtime_error("Extrude: non-OCCT shape implementation");

    if (fo->Occt().ShapeType() != TopAbs_FACE) return face;
    gp_Vec dz(0, 0, height);

    TopoDS_Shape s = BRepPrimAPI_MakePrism(TopoDS::Face(fo->Occt()), dz).Shape();
    return WrapOcctShape(s);
}

}  // namespace construct
}  // namespace ccad
