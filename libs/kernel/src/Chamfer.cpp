#include <BRepFilletAPI_MakeChamfer.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <ccad/base/Logger.hpp>

#include "ccad/base/Exception.hpp"
#include "ccad/base/Status.hpp"
#include "internal/geom/ShapeHelper.hpp"

namespace ccad::feature {

Shape ChamferAll(const Shape& s, double distanceMm) {
    if (distanceMm <= 0.0) throw Exception("Distance must be > 0", Status::ERROR_OCCT);
    auto os = ShapeAsOcct(s);

    auto occtShape = os->Occt();

    if (!os) throw std::runtime_error("Fillet: non-OCCT shape implementation");

    try {
        TopoDS_Shape in = ccad::FixIfNeeded(occtShape);

        BRepFilletAPI_MakeChamfer mk(in);

        // Edge → Faces-Mapping
        TopTools_IndexedDataMapOfShapeListOfShape edge2faces;
        TopExp::MapShapesAndAncestors(in, TopAbs_EDGE, TopAbs_FACE, edge2faces);

        int added = 0;
        for (int i = 1; i <= edge2faces.Extent(); ++i) {
            const TopoDS_Edge& e = TopoDS::Edge(edge2faces.FindKey(i));
            const TopTools_ListOfShape& faces = edge2faces.FindFromIndex(i);
            if (!faces.IsEmpty()) {
                const TopoDS_Face& f = TopoDS::Face(faces.First());
                try {
                    mk.Add(distanceMm, distanceMm, e, f);
                    ++added;
                } catch (const Standard_Failure& e) {
                    LOG(ERROR) << "[chamfer] Problem during adding: " << e.GetMessageString();
                }
            }
        }

        if (added == 0) {
            LOG(ERROR) << "[chamfer] no applicable edges, returning original.\n";
            return s;
        }

        mk.Build();
        if (!mk.IsDone()) {
            LOG(ERROR) << "[chamfer] mk.IsDone() == false, returning original.\n";
            return s;
        }

        TopoDS_Shape out = mk.Shape();
        if (out.IsNull()) {
            LOG(ERROR) << "[chamfer] result is null, returning original.\n";
            return s;
        }
        return WrapOcctShape(out);
    } catch (const Standard_Failure& e) {
        LOG(ERROR) << "[chamfer] OpenCascade error: " << e.GetMessageString() << "\n";
        return s;
    } catch (...) {
        LOG(ERROR) << "[chamfer] unknown error.\n";
        return s;
    }
}

}  // namespace ccad::feature
