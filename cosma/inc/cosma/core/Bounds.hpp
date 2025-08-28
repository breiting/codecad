#pragma once
#include <glm/glm.hpp>

struct AABB {
    glm::vec3 min{0.0f}, max{0.0f};
    bool valid{false};

    /// Reset to invalid state
    void Reset() {
        valid = false;
        min = max = glm::vec3(0.0f);
    }

    /// Include a point into the box
    void Expand(const glm::vec3& p) {
        if (!valid) {
            min = max = p;
            valid = true;
            return;
        }
        min = glm::min(min, p);
        max = glm::max(max, p);
    }

    /// Include another AABB
    void Include(const AABB& b) {
        if (!b.valid) return;
        if (!valid) {
            *this = b;
            return;
        }
        min = glm::min(min, b.min);
        max = glm::max(max, b.max);
    }

    glm::vec3 Size() const {
        return valid ? (max - min) : glm::vec3(0);
    }
    glm::vec3 Center() const {
        return valid ? (0.5f * (min + max)) : glm::vec3(0);
    }
};

/// Transform a local AABB by a model matrix and return its world-space AABB.
AABB TransformAABB(const glm::mat4& M, const AABB& local);
