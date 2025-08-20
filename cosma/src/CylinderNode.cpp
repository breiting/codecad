#include <cmath>
#include <glm/glm.hpp>
#include <material/FlatShadedMaterial.hpp>
#include <scene/CylinderNode.hpp>

CylinderNode::CylinderNode(float radius, float height, int segments) {
    auto geo = std::make_unique<Mesh>();
    auto mat = std::make_shared<FlatShadedMaterial>();

    const float halfHeight = height / 2.0f;
    auto vertexCounter = 0;
    glm::vec3 col = {0.0, 0.0, 0.0};

    for (int i = 0; i < segments; ++i) {
        float angle1 = 2.0f * glm::pi<float>() * i / segments;
        float angle2 = 2.0f * glm::pi<float>() * (i + 1) / segments;

        glm::vec3 p1 = glm::vec3(radius * cos(angle1), radius * sin(angle1), -halfHeight);
        glm::vec3 p2 = glm::vec3(radius * cos(angle2), radius * sin(angle2), -halfHeight);
        glm::vec3 p3 = glm::vec3(radius * cos(angle2), radius * sin(angle2), +halfHeight);
        glm::vec3 p4 = glm::vec3(radius * cos(angle1), radius * sin(angle1), +halfHeight);

        glm::vec3 normal = glm::normalize(glm::cross(p2 - p1, p4 - p1));

        geo->AddVertex(Vertex(p1, normal, col));
        geo->AddVertex(Vertex(p2, normal, col));
        geo->AddVertex(Vertex(p4, normal, col));
        geo->AddTriangle(vertexCounter, vertexCounter + 1, vertexCounter + 2);
        vertexCounter += 3;

        geo->AddVertex(Vertex(p2, normal, col));
        geo->AddVertex(Vertex(p3, normal, col));
        geo->AddVertex(Vertex(p4, normal, col));
        geo->AddTriangle(vertexCounter, vertexCounter + 1, vertexCounter + 2);
        vertexCounter += 3;
    }

    // ----- Top -----
    glm::vec3 topCenter = glm::vec3(0, 0, +halfHeight);
    for (int i = 0; i < segments; ++i) {
        float angle1 = 2.0f * glm::pi<float>() * i / segments;
        float angle2 = 2.0f * glm::pi<float>() * (i + 1) / segments;

        glm::vec3 p1 = glm::vec3(radius * cos(angle1), radius * sin(angle1), +halfHeight);
        glm::vec3 p2 = glm::vec3(radius * cos(angle2), radius * sin(angle2), +halfHeight);

        glm::vec3 normal = glm::vec3(0, 0, 1);

        geo->AddVertex(Vertex(topCenter, normal, col));
        geo->AddVertex(Vertex(p1, normal, col));
        geo->AddVertex(Vertex(p2, normal, col));
        geo->AddTriangle(vertexCounter, vertexCounter + 1, vertexCounter + 2);
        vertexCounter += 3;
    }

    // ----- Bottom -----
    glm::vec3 bottomCenter = glm::vec3(0, 0, -halfHeight);
    for (int i = 0; i < segments; ++i) {
        float angle1 = 2.0f * glm::pi<float>() * i / segments;
        float angle2 = 2.0f * glm::pi<float>() * (i + 1) / segments;

        glm::vec3 p1 = glm::vec3(radius * cos(angle1), radius * sin(angle1), -halfHeight);
        glm::vec3 p2 = glm::vec3(radius * cos(angle2), radius * sin(angle2), -halfHeight);

        glm::vec3 normal = glm::vec3(0, 0, -1);

        geo->AddVertex(Vertex(bottomCenter, normal, col));
        geo->AddVertex(Vertex(p2, normal, col));
        geo->AddVertex(Vertex(p1, normal, col));
        geo->AddTriangle(vertexCounter, vertexCounter + 1, vertexCounter + 2);
        vertexCounter += 3;
    }
    m_Renderable = std::make_shared<Renderable>();
    m_Renderable->mesh = std::move(geo);
    m_Renderable->material = mat;
}
