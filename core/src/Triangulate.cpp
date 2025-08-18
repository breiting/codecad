// src/geo/Triangulate.cpp
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <cmath>
#include <gp_Trsf.hxx>

#include "geo/Triangulate.hpp"

namespace geo {

TriMesh TriangulateShape(const TopoDS_Shape& shape, double defl, double angDeg, bool parallel) {
    TriMesh out;

    if (shape.IsNull()) return out;

    // 1) Meshing (falls noch nicht vorhanden)
    //    last arg 'true' = relative deflection (stabiler bei unterschiedlichen Größen)
    BRepMesh_IncrementalMesh mesher(shape, defl, parallel, angDeg * M_PI / 180.0, true);

    // 2) Faces iterieren und Triangulation einsammeln
    for (TopExp_Explorer ex(shape, TopAbs_FACE); ex.More(); ex.Next()) {
        const TopoDS_Face face = TopoDS::Face(ex.Current());
        TopLoc_Location loc;
        Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, loc);
        if (tri.IsNull()) continue;

        const gp_Trsf trsf = loc.Transformation();

        // Nodes liegen 1-basiert im Array [Lower..Upper]
        const TColgp_Array1OfPnt& nodes = tri->Nodes();
        const Poly_Array1OfTriangle& tris = tri->Triangles();

        // Merke Basisindex für Indizes in diesem Chunk
        const int base = static_cast<int>(out.positions.size());

        // 2a) alle Knoten transformieren und anhängen
        for (Standard_Integer i = nodes.Lower(); i <= nodes.Upper(); ++i) {
            gp_Pnt p = nodes(i).Transformed(trsf);
            out.positions.emplace_back(static_cast<float>(p.X()), static_cast<float>(p.Y()), static_cast<float>(p.Z()));
        }

        const bool reversed = (face.Orientation() == TopAbs_REVERSED);

        // 2b) Dreiecke einsammeln (Knotenindizes sind 1-basiert)
        for (Standard_Integer i = tris.Lower(); i <= tris.Upper(); ++i) {
            Standard_Integer n1, n2, n3;
            tris(i).Get(n1, n2, n3);
            // auf 0-basierte, shape-weite Indizes abbilden
            const int lower = nodes.Lower();
            unsigned i1 = base + (n1 - lower);
            unsigned i2 = base + (n2 - lower);
            unsigned i3 = base + (n3 - lower);

            if (reversed) std::swap(i2, i3);  // Orientierung korrigieren

            out.indices.push_back(i1);
            out.indices.push_back(i2);
            out.indices.push_back(i3);
        }
    }

    return out;
}

}  // namespace geo
