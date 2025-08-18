#pragma once

#include <scene/ShapeNode.hpp>

class CylinderNode : public ShapeNode {
   public:
    CylinderNode(float radius = 1.0, float height = 1.0, int segments = 32);
    ~CylinderNode() = default;
};
