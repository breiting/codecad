#pragma once

#include <scene/ShapeNode.hpp>

class ConeNode : public ShapeNode {
   public:
    ConeNode(float radius = 1.0, float height = 1.0, int segments = 32);
    ~ConeNode() = default;
};
