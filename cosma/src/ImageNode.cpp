#include <material/FlatShadedMaterial.hpp>
#include <scene/ImageNode.hpp>

ImageNode::ImageNode(float width, float height) {
    float halfWidth = width * 0.5f;

    auto geo = std::make_unique<Mesh>();
    auto mat = std::make_shared<FlatShadedMaterial>();

    geo->AddVertex({glm::vec3(-halfWidth, 0.0f, 0.0f), glm::vec3(0, 1, 0), glm::vec3(1, 1, 1), glm::vec2(0, 0)});
    geo->AddVertex({glm::vec3(halfWidth, 0.0f, 0.0f), glm::vec3(0, 1, 0), glm::vec3(1, 1, 1), glm::vec2(1, 0)});
    geo->AddVertex({glm::vec3(halfWidth, 0.0f, height), glm::vec3(0, 1, 0), glm::vec3(1, 1, 1), glm::vec2(1, 1)});
    geo->AddVertex({glm::vec3(-halfWidth, 0.0f, height), glm::vec3(0, 1, 0), glm::vec3(1, 1, 1), glm::vec2(0, 1)});

    geo->AddTriangle(0, 1, 2);
    geo->AddTriangle(2, 3, 0);

    m_Renderable = std::make_shared<Renderable>();
    m_Renderable->mesh = std::move(geo);
    m_Renderable->material = mat;
}
