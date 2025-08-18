#pragma once
#include <TopoDS_Shape.hxx>
#include <glm/vec3.hpp>
#include <vector>

namespace geo {

struct TriMesh {
    std::vector<glm::vec3> positions;
    std::vector<unsigned> indices;  // 0-based, triples (CCW)
};

/// Triangulate a shape for real-time viewing.
/// @param defl  linear deflection in model units (mm) – kleiner = feiner
/// @param angDeg angular deflection in degrees – kleiner = feiner
TriMesh TriangulateShape(const TopoDS_Shape& shape, double defl = 0.2, double angDeg = 20.0, bool parallel = true);
