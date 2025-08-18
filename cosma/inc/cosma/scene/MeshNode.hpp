#pragma once

#include <scene/ShapeNode.hpp>

class MeshNode : public ShapeNode {
   public:
    MeshNode(std::unique_ptr<Mesh> mesh);
    ~MeshNode() = default;

    void SetMesh(std::unique_ptr<Mesh> mesh);
};
