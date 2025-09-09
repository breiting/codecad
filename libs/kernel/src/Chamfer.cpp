#include "ccad/feature/Chamfer.hpp"

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepGProp.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <ccad/base/Logger.hpp>
#include <cmath>

#include "ccad/base/Exception.hpp"
#include "ccad/base/Math.hpp"
#include "ccad/base/Status.hpp"
#include "ccad/construct/Revolve.hpp"
#include "ccad/sketch/SketchProfiles.hpp"
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

Shape ChamferCutterRadial(double diameter, const ChamferRadialSpec& spec) {
    const double xLength = spec.length * std::cos(DegToRad(spec.angleDeg));
    const double zLength = spec.length * std::sin(DegToRad(spec.angleDeg));

    double xStart = 0.0;
    if (spec.type == ChamferRadialType::External) {
        xStart = diameter * 0.5 - xLength;
    } else if (spec.type == ChamferRadialType::Internal) {
        xStart = -diameter * 0.5 - xLength;
    } else {
        Exception("Unknown ChamferRadialType");
    }
    std::vector<Vec2> triangle{
        {xStart, 0},
        {xStart + xLength, 0},
        {xStart + xLength, zLength},
    };
    auto profile = sketch::ProfileXZ(triangle, true);

    return construct::RevolveZ(profile, 360);
}

/** Map all edges of a shape to a stable 1-based index. */
static TopTools_IndexedMapOfShape BuildEdgeMap(const TopoDS_Shape& shape) {
    TopTools_IndexedMapOfShape emap;
    TopExp::MapShapes(shape, TopAbs_EDGE, emap);
    return emap;
}

/** Resolve a zero-based EdgeRef.index to a TopoDS_Edge using an IndexedMap. */
static TopoDS_Edge GetEdgeByIndex(const TopTools_IndexedMapOfShape& emap, std::size_t zeroBasedIndex) {
    Standard_Integer oneBased = static_cast<Standard_Integer>(zeroBasedIndex + 1);
    if (oneBased < 1 || oneBased > emap.Extent()) throw std::out_of_range("Chamfer: edge index out of range");
    return TopoDS::Edge(emap.FindKey(oneBased));
}

/** Build edge→faces adjacency for a shape. */
static TopTools_IndexedDataMapOfShapeListOfShape BuildEdgeToFaces(const TopoDS_Shape& shape) {
    TopTools_IndexedDataMapOfShapeListOfShape map;
    TopExp::MapShapesAndAncestors(shape, TopAbs_EDGE, TopAbs_FACE, map);
    return map;
}

/** If input is a planar WIRE, promote to FACE for 2D chamfering. */
static TopoDS_Shape EnsureFaceIfWire(const TopoDS_Shape& in) {
    if (in.ShapeType() == TopAbs_WIRE) {
        TopoDS_Wire w = TopoDS::Wire(in);
        return BRepBuilderAPI_MakeFace(w).Face();
    }
    return in;
}

Shape Chamfer(const Shape& s, const select::EdgeSet& edgeSet, double distanceMm) {
    if (distanceMm <= 0.0) return s;

    auto os = ShapeAsOcct(s);
    if (!os) throw std::runtime_error("Chamfer: non-OCCT shape implementation");
    TopoDS_Shape shape = EnsureFaceIfWire(os->Occt());

    // Pre-build maps for fast lookups
    TopTools_IndexedMapOfShape edgeMap = BuildEdgeMap(shape);
    TopTools_IndexedDataMapOfShapeListOfShape edge2faces = BuildEdgeToFaces(shape);

    BRepFilletAPI_MakeChamfer mkChamfer(shape);

    // Add a symmetric Distance/Distance chamfer for each selected edge.
    for (const auto& er : edgeSet.items()) {
        TopoDS_Edge e;
        try {
            e = GetEdgeByIndex(edgeMap, er.index);
        } catch (...) {
            continue;  // skip invalid index
        }

        // Fetch an adjacent face; for solids there are usually 2, for open shells 1.
        if (!edge2faces.Contains(e)) {
            // No adjacent face information; skip this edge.
            continue;
        }
        const TopTools_ListOfShape& lst = edge2faces.FindFromKey(e);
        if (lst.IsEmpty()) {
            continue;
        }
        // Use the first adjacent face (robust default).
        TopoDS_Face f = TopoDS::Face(lst.First());

        // Symmetric D1=D2 chamfer
        mkChamfer.Add(distanceMm, distanceMm, e, f);
    }

    mkChamfer.Build();
    if (!mkChamfer.IsDone()) {
        // Graceful fallback: return original shape on failure
        return s;
    }
    return WrapOcctShape(mkChamfer.Shape());
}

}  // namespace ccad::feature
