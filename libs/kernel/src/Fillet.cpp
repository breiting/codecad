#include <BRepFilletAPI_MakeFillet.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <ccad/base/Logger.hpp>

#include "ccad/base/Exception.hpp"
#include "ccad/base/Status.hpp"
#include "internal/geom/ShapeHelper.hpp"

namespace ccad::feature {

Shape FilletAll(const Shape& s, double radiusMm) {
    if (radiusMm <= 0.0) throw Exception("Radius must be > 0", Status::ERROR_OCCT);
    auto os = ShapeAsOcct(s);

    auto occtShape = os->Occt();

    if (!os) throw std::runtime_error("Fillet: non-OCCT shape implementation");

    try {
        TopoDS_Shape in = ccad::FixIfNeeded(occtShape);

        BRepFilletAPI_MakeFillet mk(in);

        int edge_count = 0;
        for (TopExp_Explorer ex(in, TopAbs_EDGE); ex.More(); ex.Next()) {
            const TopoDS_Edge& e = TopoDS::Edge(ex.Current());
            mk.Add(radiusMm, e);
            ++edge_count;
        }

        if (edge_count == 0) {
            LOG(ERROR) << "[fillet] no edges on shape, returning original.";
            return s;
        }

        mk.Build();
        if (!mk.IsDone()) {
            LOG(ERROR) << "[fillet] mk.IsDone() == false, returning original.";
            std::cerr << "";
            return s;
        }

        TopoDS_Shape out = mk.Shape();
        if (out.IsNull()) {
            LOG(ERROR) << "[fillet] result is null, returning original";
            return s;
        }

        return WrapOcctShape(out);
    } catch (const Standard_Failure& e) {
        LOG(ERROR) << "[fillet] OpenCascade error:" << e.GetMessageString();
        return s;
    } catch (...) {
        LOG(ERROR) << "[fillet] unknown error";
        return s;
    }
}

}  // namespace ccad::feature
