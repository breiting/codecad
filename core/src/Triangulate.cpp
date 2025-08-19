#include "geometry/Triangulate.hpp"

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangle.hxx>
#include <Poly_Triangulation.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <gp_Trsf.hxx>

namespace geometry {

TriMesh TriangulateShape(const TopoDS_Shape& shape, double defl, double angDeg, bool parallel) {
    TriMesh out;
    if (shape.IsNull()) return out;

    BRepMesh_IncrementalMesh mesher(shape, defl, parallel, angDeg * M_PI / 180.0, true);

    for (TopExp_Explorer ex(shape, TopAbs_FACE); ex.More(); ex.Next()) {
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
            out.positions.emplace_back(static_cast<float>(p.X()), static_cast<float>(p.Y()), static_cast<float>(p.Z()));
        }

        // collect triangles
        const int nbTris = tri->NbTriangles();
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

    return out;
}

}  // namespace geometry
