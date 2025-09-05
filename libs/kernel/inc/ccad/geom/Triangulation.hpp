#pragma once

#include <ccad/base/Math.hpp>
#include <ccad/base/Shape.hpp>

namespace ccad {
namespace geom {

/** \brief 3D triangular mesh. */
class TriMesh {
   public:
    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    std::vector<unsigned> indices;

    friend std::ostream& operator<<(std::ostream& os, const TriMesh& mesh);
};

/** \brief Triangulation parameters */
struct TriangulationParams {
    double linearDeflection = 0.2;
    double angularDeflectionDeg = 20.0;
    bool parallel = true;
};

TriMesh Triangulate(const Shape& s, const TriangulationParams& p = {});

}  // namespace geom
}  // namespace ccad
