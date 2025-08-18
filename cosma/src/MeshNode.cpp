#include <geometry/Vertex.hpp>
#include <glm/glm.hpp>
#include <material/FlatShadedMaterial.hpp>
#include <scene/MeshNode.hpp>

MeshNode::MeshNode(std::unique_ptr<Mesh> mesh) {
    auto mat = std::make_shared<FlatShadedMaterial>();

    m_Renderable = std::make_shared<Renderable>();
    m_Renderable->mesh = std::move(mesh);
    m_Renderable->material = mat;
}

void MeshNode::SetMesh(std::unique_ptr<Mesh> mesh) {
    m_Renderable->mesh = std::move(mesh);
}
