#pragma once

#include <glm/glm.hpp>
#include <pure/PureMath.hpp>
#include <pure/PureTypes.hpp>

using namespace pure;

namespace pure::math {

/// Simple point to point distance
float DistancePointPoint(glm::vec3 a, glm::vec3 b);

/// Simple point to edge distance
float DistancePointEdge(glm::vec3 p, const Edge& e);

/// Simple edge to edge distance (makes most sense with parallel lines)
float DistanceEdgeEdgeSimple(const Edge& e1, const Edge& e2, float angleDegTol = 5.0f);

}  // namespace pure::math
