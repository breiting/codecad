#include <ccad/geom/Triangulation.hpp>

// OCCT
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <TopAbs.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>

#include "ccad/base/Logger.hpp"
#include "ccad/base/Math.hpp"
#include "internal/geom/OcctShape.hpp"
#include "internal/geom/ShapeHelper.hpp"

namespace ccad::geom {

std::ostream& operator<<(std::ostream& os, const TriMesh& mesh) {
    os << "TriMesh summary:\n";
    os << "  vertices:  " << mesh.positions.size() << "\n";
    os << "  normals:   " << mesh.normals.size() << "\n";
    os << "  triangles: " << mesh.indices.size() / 3 << "\n\n";

    auto limitVerts = std::min<size_t>(100, mesh.positions.size());
    os << "  Vertices (first " << limitVerts << "):\n";
    for (size_t i = 0; i < limitVerts; ++i) {
        const auto& v = mesh.positions[i];
        os << "    [" << i << "] " << std::fixed << std::setprecision(3) << v.x << ", " << v.y << ", " << v.z << "\n";
    }

    auto limitNorms = std::min<size_t>(100, mesh.normals.size());
    if (limitNorms > 0) {
        os << "\n  Normals (first " << limitNorms << "):\n";
        for (size_t i = 0; i < limitNorms; ++i) {
            const auto& n = mesh.normals[i];
            os << "    [" << i << "] " << std::fixed << std::setprecision(3) << n.x << ", " << n.y << ", " << n.z
               << "\n";
        }
    }

    auto limitTris = std::min<size_t>(50, mesh.indices.size() / 3);
    os << "\n  Triangles (first " << limitTris << "):\n";
    for (size_t i = 0; i < limitTris; ++i) {
        size_t idx = i * 3;
        os << "    [" << i << "] " << mesh.indices[idx] << ", " << mesh.indices[idx + 1] << ", "
           << mesh.indices[idx + 2] << "\n";
    }

    if (mesh.positions.size() > limitVerts || mesh.normals.size() > limitNorms ||
        (mesh.indices.size() / 3) > limitTris) {
        os << "\n  ... truncated ...\n";
    }

    return os;
}

TriMesh Triangulate(const Shape& shape, const geom::TriangulationParams& p) {
    auto s = ShapeAsOcct(shape);
    if (!s) throw std::runtime_error("Triangulate: non-OCCT shape implementation");

    auto os = s->Occt();

    BRepMesh_IncrementalMesh mesher(os, p.linearDeflection, false, DegToRad(p.angularDeflectionDeg), p.parallel);
    mesher.Perform();

    TriMesh out;
    int totalNumTriangles{0};

    for (TopExp_Explorer ex(os, TopAbs_FACE); ex.More(); ex.Next()) {
        const TopoDS_Face face = TopoDS::Face(ex.Current());
        TopLoc_Location loc;
        Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, loc);
        if (tri.IsNull()) continue;

        const gp_Trsf trsf = loc.Transformation();
        const bool reversed = (face.Orientation() == TopAbs_REVERSED);

        const int base = static_cast<int>(out.positions.size());

        // collect nodes
        const int nbNodes = tri->NbNodes();
        out.positions.reserve(out.positions.size() + static_cast<size_t>(nbNodes));
        for (int i = 1; i <= nbNodes; ++i) {
            gp_Pnt p = tri->Node(i).Transformed(trsf);
            out.positions.emplace_back(Vec3(p.X(), p.Y(), p.Z()));
        }

        // collect triangles
        const int nbTris = tri->NbTriangles();
        totalNumTriangles += nbTris;
        out.indices.reserve(out.indices.size() + static_cast<size_t>(nbTris) * 3);
        for (int i = 1; i <= nbTris; ++i) {
            const Poly_Triangle t = tri->Triangle(i);
            int n1, n2, n3;
            t.Get(n1, n2, n3);  // 1-basiert

            unsigned i1 = base + (n1 - 1);
            unsigned i2 = base + (n2 - 1);
            unsigned i3 = base + (n3 - 1);
            if (reversed) std::swap(i2, i3);

            out.indices.push_back(i1);
            out.indices.push_back(i2);
            out.indices.push_back(i3);
        }
    }
    LOG(INFO) << "Triangulation::NumTriangles: " << totalNumTriangles;

    return out;
}

}  // namespace ccad::geom
