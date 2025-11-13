#include <BRepAlgoAPI_Cut.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <ccad/geom/Poisson.hpp>
#include <gp_Ax2.hxx>
#include <gp_Pnt.hxx>

#include "ccad/base/Exception.hpp"
#include "ccad/base/Status.hpp"
#include "internal/geom/ShapeHelper.hpp"

namespace ccad::geom {

TopoDS_Compound makeCompound(const std::vector<TopoDS_Shape>& holes) {
    TopoDS_Compound comp;
    BRep_Builder builder;
    builder.MakeCompound(comp);

    for (const auto& h : holes) {
        builder.Add(comp, h);
    }
    return comp;
}

Shape Poisson(const PoissonDiskSpec& spec, double thickness, const PoissonResult& points) {
    if (spec.height <= 0 || spec.width <= 0) throw Exception("Poisson: sizes must be > 0", Status::ERROR_OCCT);

    TopoDS_Shape plate = BRepPrimAPI_MakeBox(spec.width, thickness, spec.height);

    std::vector<TopoDS_Shape> holes;
    holes.reserve(points.points.size());

    for (const auto& p : points.points) {
        TopoDS_Shape cyl = BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(p.x, 0, p.y), gp::DY()), p.r, thickness);
        holes.push_back(cyl);
    }
    TopoDS_Compound comp = makeCompound(holes);
    plate = BRepAlgoAPI_Cut(plate, comp);

    return WrapOcctShape(plate);
}

}  // namespace ccad::geom
