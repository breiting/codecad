#include <glm/glm.hpp>
#include <material/FlatShadedMaterial.hpp>
#include <memory>
#include <scene/TriangleNode.hpp>

TriangleNode::TriangleNode() {
    auto geo = std::make_unique<Mesh>();
    auto mat = std::make_shared<FlatShadedMaterial>();

    Vertex v1({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f});
    Vertex v2({1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f});
    Vertex v3({0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f});

    geo->AddVertex(v1);
    geo->AddVertex(v2);
    geo->AddVertex(v3);

    geo->AddTriangle(0, 1, 2);

    m_Renderable = std::make_shared<Renderable>();
    m_Renderable->mesh = std::move(geo);
    m_Renderable->material = mat;
}
