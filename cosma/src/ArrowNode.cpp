#include <geometry/LineSet.hpp>
#include <glm/gtx/quaternion.hpp>
#include <material/LineSetMaterial.hpp>
#include <scene/ArrowNode.hpp>

ArrowNode::ArrowNode(float length, float headLength) : m_Length(length), m_HeadLength(headLength) {
    auto geo = std::make_unique<LineSet>();
    auto mat = std::make_shared<LineSetMaterial>();

    Vertex v1, v2, v3, v4;
    v1 = Vertex({0.0, 0.0, 0.0}, glm::vec3(1, 0, 0), glm::vec3(1.0, 0.0, 0.0));
    v2 = Vertex({0.0, 0.0, length}, glm::vec3(1, 0, 0), glm::vec3(1.0, 0.0, 0.0));
    v3 = Vertex({-headLength, 0.0, length - headLength}, glm::vec3(1, 0, 0), {1.0, 0.0, 0.0});
    v4 = Vertex({headLength, 0.0, length - headLength}, glm::vec3(1, 0, 0), {1.0, 0.0, 0.0});

    geo->AddVertex(v1);
    geo->AddVertex(v2);

    geo->AddVertex(v2);
    geo->AddVertex(v3);

    geo->AddVertex(v2);
    geo->AddVertex(v4);

    m_Renderable = std::make_shared<Renderable>();
    m_Renderable->lines = std::move(geo);
    m_Renderable->material = mat;
}

void ArrowNode::SetDirection(const glm::vec3& direction) {
    glm::vec3 up = glm::vec3(0, 1, 0);
    glm::quat rot = glm::rotation(glm::vec3(0, 0, 1), glm::normalize(direction));
    SetRotation(rot);
}
