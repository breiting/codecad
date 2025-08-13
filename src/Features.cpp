#include "geo/Features.hpp"

#include <BRepCheck_Analyzer.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <ShapeFix_Shape.hxx>
#include <Standard_Failure.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <iostream>

namespace geo {

static bool IsValid(const TopoDS_Shape& s) {
    BRepCheck_Analyzer ana(s);
    return ana.IsValid();
}

static TopoDS_Shape FixIfNeeded(const TopoDS_Shape& s) {
    if (IsValid(s)) return s;
    Handle(ShapeFix_Shape) fixer = new ShapeFix_Shape(s);
    fixer->Perform();
    return fixer->Shape();
}

ShapePtr FilletAll(const ShapePtr& s, double r) {
    if (!s || r <= 0.0) return s;
    try {
        TopoDS_Shape in = FixIfNeeded(s->Get());

        BRepFilletAPI_MakeFillet mk(in);

        int edge_count = 0;
        for (TopExp_Explorer ex(in, TopAbs_EDGE); ex.More(); ex.Next()) {
            const TopoDS_Edge& e = TopoDS::Edge(ex.Current());
            // Optional: only selection edges (filter) - future idea
            mk.Add(r, e);
            ++edge_count;
        }

        if (edge_count == 0) {
            std::cerr << "[fillet] no edges on shape, skipping.\n";
            return s;
        }

        mk.Build();
        if (!mk.IsDone()) {
            std::cerr << "[fillet] mk.IsDone() == false, returning original.\n";
            return s;
        }

        TopoDS_Shape out = mk.Shape();
        if (out.IsNull()) {
            std::cerr << "[fillet] result is null, returning original.\n";
            return s;
        }

        return std::make_shared<Shape>(out);
    } catch (const Standard_Failure& e) {
        std::cerr << "[fillet] OpenCascade error: " << e.GetMessageString() << "\n";
        return s;
    } catch (...) {
        std::cerr << "[fillet] unknown error.\n";
        return s;
    }
}

ShapePtr ChamferAll(const ShapePtr& s, double d) {
    if (!s || d <= 0.0) return s;
    try {
        TopoDS_Shape in = FixIfNeeded(s->Get());

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
                    mk.Add(d, d, e, f);
                    ++added;
                } catch (const Standard_Failure&) {
                }
            }
        }

        if (added == 0) {
            std::cerr << "[chamfer] no applicable edges, returning original.\n";
            return s;
        }

        mk.Build();
        if (!mk.IsDone()) {
            std::cerr << "[chamfer] mk.IsDone() == false, returning original.\n";
            return s;
        }

        TopoDS_Shape out = mk.Shape();
        if (out.IsNull()) {
            std::cerr << "[chamfer] result is null, returning original.\n";
            return s;
        }
        return std::make_shared<Shape>(out);
    } catch (const Standard_Failure& e) {
        std::cerr << "[chamfer] OpenCascade error: " << e.GetMessageString() << "\n";
        return s;
    } catch (...) {
        std::cerr << "[chamfer] unknown error.\n";
        return s;
    }
}

}  // namespace geo
