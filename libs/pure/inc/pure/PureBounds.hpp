#pragma once
#include <glm/glm.hpp>

/**
 * @brief Axis-aligned scene bounds (world space).
 */
struct PureBounds {
    glm::vec3 min{0.0f};
    glm::vec3 max{0.0f};
    bool valid{false};

    glm::vec3 Center() const {
        return 0.5f * (min + max);
    }
    glm::vec3 Size() const {
        return (max - min);
    }
};
