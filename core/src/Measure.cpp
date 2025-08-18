#include "geometry/Measure.hpp"

#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <TopoDS_Shape.hxx>

namespace geometry {

BBox ComputeBBox(const ShapePtr& s, bool useTriangulation) {
    BBox out;
    if (!s || s->Get().IsNull()) return out;

    Bnd_Box bb;
    bb.SetGap(0.0);
    BRepBndLib::Add(s->Get(), bb, useTriangulation ? true : false);

    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bb.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    if (!bb.IsVoid()) {
        out.minX = xmin;
        out.minY = ymin;
        out.minZ = zmin;
        out.maxX = xmax;
        out.maxY = ymax;
        out.maxZ = zmax;
        out.isValid = true;
    }
    return out;
}

}  // namespace geometry
