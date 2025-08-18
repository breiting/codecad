#pragma once

#include <scene/ShapeNode.hpp>

class BoxNode : public ShapeNode {
   public:
    BoxNode(float xSize = 1.0, float ySize = 1.0, float zSize = 1.0);
    ~BoxNode() = default;
};
