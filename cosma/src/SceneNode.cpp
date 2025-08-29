#include <glm/gtx/euler_angles.hpp>
#include <glm/trigonometric.hpp>
#include <scene/SceneNode.hpp>

SceneNode::SceneNode(const std::string& name) : m_Name(name) {
}

void SceneNode::SetName(const std::string& name) {
    m_Name = name;
}
std::string SceneNode::GetName() const {
    return m_Name;
}

void SceneNode::SetPosition(const glm::vec3& position) {
    m_Transform.position = position;
}

void SceneNode::SetScale(const glm::vec3& scale) {
    m_Transform.scale = scale;
}

void SceneNode::SetRotationEuler(const glm::vec3& rotation) {
    m_Transform.rotation = glm::quat(rotation);
}

void SceneNode::SetRotation(const glm::quat& rotation) {
    m_Transform.rotation = rotation;
}

void SceneNode::Move(const glm::vec3& delta) {
    m_Transform.position += delta;
}

glm::mat4 SceneNode::WorldMatrix() const {
    return m_Transform.GetTransformationMatrix();
}
