#pragma once

#include <glm/glm.hpp>
#include <pure/PureMath.hpp>
#include <pure/PureTypes.hpp>

using namespace pure;

namespace pure::math {

/// Closest points between segments (Robust; returns squared distance)
float ClosestPointsOnSegments(const Edge& s0, const Edge& s1, glm::vec3& c0, glm::vec3& c1);

/// Project a vector onto a constraint axis and return its signed length
float ProjectedLength(glm::vec3 v, ConstraintAxis a);

/// Apply axis constraint to turn any p1 into constrained p1' wrt p0.
glm::vec3 Constrain(const glm::vec3& p0, const glm::vec3& p1, ConstraintAxis a);

/// Simple point to point distance
float DistancePointPoint(glm::vec3 a, glm::vec3 b);

/// Simple point to edge distance
float DistancePointEdge(glm::vec3 p, const Edge& e);

/// Simple edge to edge distance (makes most sense with parallel lines)
float DistanceEdgeEdgeSimple(const Edge& e1, const Edge& e2, float angleDegTol = 5.0f);

}  // namespace pure::math
