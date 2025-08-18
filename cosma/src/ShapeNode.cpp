#include <glm/trigonometric.hpp>
#include <scene/ShapeNode.hpp>

ShapeNode::ShapeNode() : m_RigidBody(nullptr) {
}

void ShapeNode::SetMaterial(std::shared_ptr<Material> material) {
    if (m_Renderable) {
        m_Renderable->material = material;
    }
}

void ShapeNode::Update(float deltaTime) {
    if (m_RigidBody) {
        m_RigidBody->Update(deltaTime);
        m_Transform.position = m_RigidBody->GetPosition();
    }
}

void ShapeNode::Render(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                       std::shared_ptr<Light> light) {
    glm::mat4 worldTransform = model * m_Transform.GetTransformationMatrix();

    if (m_Renderable) {
        m_Renderable->Render(worldTransform, view, projection, light);
    }
}

void ShapeNode::AttachRigidBody(std::shared_ptr<RigidBodyComponent> rigidBody) {
    m_RigidBody = rigidBody;
    if (m_RigidBody) {
        m_Transform.position = m_RigidBody->GetPosition();
    }
}

bool ShapeNode::HasPhysics() const {
    return m_RigidBody != nullptr;
}
