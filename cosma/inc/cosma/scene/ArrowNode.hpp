#pragma once

#include <scene/ShapeNode.hpp>

class ArrowNode : public ShapeNode {
   public:
    ArrowNode(float length = 1.0f, float headLength = 0.2f);
    ~ArrowNode() = default;

    void SetDirection(const glm::vec3& direction);
};
