#include <cmath>
#include <glm/glm.hpp>
#include <material/FlatShadedMaterial.hpp>
#include <scene/ConeNode.hpp>

ConeNode::ConeNode(float radius, float height, int segments) {
    auto geo = std::make_unique<Mesh>();
    auto mat = std::make_shared<FlatShadedMaterial>();

    const float halfHeight = height / 2.0f;
    glm::vec3 tip = glm::vec3(0, 0, halfHeight);
    glm::vec3 col = glm::vec3(0, 0, 0);

    // Side
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        glm::vec3 base = glm::vec3(x, y, -halfHeight);

        glm::vec3 dir = glm::normalize(glm::vec3(x, y, radius / height));
        geo->AddVertex(Vertex(base, dir, col));
        geo->AddVertex(Vertex(tip, dir, col));

        if (i > 0) {
            int baseIndex = (i - 1) * 2;
            geo->AddTriangle(baseIndex, baseIndex + 2, baseIndex + 1);
            geo->AddTriangle(baseIndex + 2, baseIndex + 3, baseIndex + 1);
        }
    }

    // Floor
    int centerIndex = geo->VertexCount();
    geo->AddVertex(Vertex(glm::vec3(0, 0, -halfHeight), glm::vec3(0, 0, -1), col));
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        geo->AddVertex(Vertex(glm::vec3(x, y, -halfHeight), glm::vec3(0, 0, -1), col));

        if (i > 0) {
            geo->AddTriangle(centerIndex, centerIndex + i + 1, centerIndex + i);
        }
    }
    m_Renderable = std::make_shared<Renderable>();
    m_Renderable->mesh = std::move(geo);
    m_Renderable->material = mat;
}
