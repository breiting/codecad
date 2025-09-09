#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepFilletAPI_MakeFillet2d.hxx>
#include <BRepGProp.hxx>
#include <BRep_Tool.hxx>
#include <GProp_GProps.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopOpeBRepTool.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <ccad/base/Logger.hpp>

#include "ccad/base/Exception.hpp"
#include "ccad/base/Logger.hpp"
#include "ccad/base/Status.hpp"
#include "ccad/select/EdgeSelector.hpp"
#include "internal/geom/ShapeHelper.hpp"

using namespace ccad::select;

namespace ccad::feature {

Shape FilletAll(const Shape& s, double radiusMm) {
    if (radiusMm <= 0.0) throw Exception("Radius must be > 0", Status::ERROR_OCCT);
    auto os = ShapeAsOcct(s);
    if (!os) throw std::runtime_error("Fillet: non-OCCT shape implementation");

    auto occtShape = os->Occt();

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

/// Map all edges of a shape into an indexed map (1-based).
inline TopTools_IndexedMapOfShape BuildEdgeMap(const TopoDS_Shape& s) {
    TopTools_IndexedMapOfShape m;
    TopExp::MapShapes(s, TopAbs_EDGE, m);
    return m;
}

/// Resolve EdgeRef.index -> TopoDS_Edge via a prebuilt map (1-based).
inline TopoDS_Edge EdgeFromIndex(const TopTools_IndexedMapOfShape& map, size_t zeroBasedIndex) {
    const int idx1 = static_cast<int>(zeroBasedIndex) + 1;      // OCCT uses 1-based indexing
    if (idx1 < 1 || idx1 > map.Extent()) return TopoDS_Edge();  // null edge
    return TopoDS::Edge(map.FindKey(idx1));
}

Shape Fillet(const Shape& in, const select::EdgeSet& edges, double radiusMm) {
    if (radiusMm <= 0.0) return in;

    auto os = ShapeAsOcct(in);
    if (!os) throw std::runtime_error("Fillet: non-OCCT shape implementation");
    const TopoDS_Shape shape = os->Occt();
    const TopAbs_ShapeEnum stype = shape.ShapeType();

    // Build edge map once for index -> TopoDS_Edge resolution
    const TopTools_IndexedMapOfShape edgeMap = BuildEdgeMap(shape);

    // ---- 2D case: Wire / Face -> MakeFace if needed, then MakeFillet2d ----
    if (stype == TopAbs_WIRE || stype == TopAbs_FACE) {
        TopoDS_Face face;
        if (stype == TopAbs_FACE) {
            face = TopoDS::Face(shape);
        } else {
            const TopoDS_Wire w = TopoDS::Wire(shape);
            BRepBuilderAPI_MakeFace mf(w);
            if (!mf.IsDone()) {
                std::cerr << "Fillet2d: cannot make face from wire\n";
                return in;
            }
            face = mf.Face();
        }

        BRepFilletAPI_MakeFillet2d mk2d(face);
        int added = 0;
        for (const auto& er : edges.items()) {
            const TopoDS_Edge E = EdgeFromIndex(edgeMap, er.index);
            if (E.IsNull()) continue;

            // iterate over edge vertices
            TopoDS_Vertex v1, v2;
            TopExp::Vertices(E, v1, v2);
            if (!v1.IsNull()) {
                try {
                    mk2d.AddFillet(v1, radiusMm);
                    ++added;
                } catch (...) {
                }
            }
            if (!v2.IsNull()) {
                try {
                    mk2d.AddFillet(v2, radiusMm);
                    ++added;
                } catch (...) {
                }
            }
        }
        if (added == 0) {
            std::cerr << "Fillet2d: no valid edges for this face\n";
            return in;
        }
        mk2d.Build();
        if (!mk2d.IsDone()) {
            std::cerr << "Fillet2d failed\n";
            return in;
        }
        return WrapOcctShape(mk2d.Shape());
    }

    // ---- 3D case: Solid / Shell / Compound ----
    BRepFilletAPI_MakeFillet mk(shape);
    int added = 0;
    for (const auto& er : edges.items()) {
        const TopoDS_Edge E = EdgeFromIndex(edgeMap, er.index);
        if (E.IsNull()) continue;
        try {
            mk.Add(radiusMm, E);  // (radius, edge) signature in OCCT 7.9
            ++added;
        } catch (...) {
            // ignore this edge
        }
    }
    if (added == 0) {
        std::cerr << "Fillet3d: no valid edges for this shape\n";
        return in;
    }

    mk.Build();
    if (!mk.IsDone()) {
        int err = 0;
        mk.StripeStatus(err);
        LOG(ERROR) << "Fillet failed. Status=" << err << " radius=" << radiusMm;
        return in;
    }
    return WrapOcctShape(mk.Shape());
}

}  // namespace ccad::feature
