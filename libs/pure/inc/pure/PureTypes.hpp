#pragma once
#include <glad.h>

#include <glm/glm.hpp>

namespace pure {

struct PureVertex {
    glm::vec3 position;
    glm::vec3 normal;
};

struct PureAabb {
    glm::vec3 min{0}, max{0};
    bool valid{false};
    void Reset() {
        valid = false;
    }
    void Expand(const glm::vec3& p) {
        if (!valid) {
            min = max = p;
            valid = true;
            return;
        }
        min = glm::min(min, p);
        max = glm::max(max, p);
    }
};

struct Edge {
    glm::vec3 a;
    glm::vec3 b;
};

enum class MeasureMode { PointToPoint, EdgeToEdge };
enum class ConstraintAxis { None, X, Y, Z };
enum class SnapType { Vertex, Edge, Face };

}  // namespace pure
