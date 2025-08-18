#pragma once

#include <geometry/Mesh.hpp>
#include <scene/ShapeNode.hpp>

class SphereNode : public ShapeNode {
   public:
    SphereNode(float radius = 1.0, int segments = 20, int rings = 20);
};
