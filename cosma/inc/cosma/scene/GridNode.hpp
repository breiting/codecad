#pragma once

#include <geometry/Mesh.hpp>
#include <scene/ShapeNode.hpp>

class GridNode : public ShapeNode {
   public:
    GridNode(float xSize, float ySize, int subdivisions, bool center = true, bool randomZ = false);
};
