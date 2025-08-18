#include <components/RigidBodyComponent.hpp>
#include <physics/core/PhysicsWorld.hpp>
#include <physics/core/RigidBody.hpp>

RigidBodyComponent::RigidBodyComponent(std::shared_ptr<RigidBody> rigidBody) : m_RigidBody(rigidBody) {
}

void RigidBodyComponent::Update(float deltaTime) {
    if (m_RigidBody) {
        m_RigidBody->Update(deltaTime);
    }
}

void RigidBodyComponent::ApplyToPhysicsWorld(PhysicsWorld& world) {
    if (m_RigidBody) {
        world.AddRigidBody(m_RigidBody);
    }
}

glm::vec3 RigidBodyComponent::GetPosition() const {
    return m_RigidBody ? m_RigidBody->GetPosition() : glm::vec3(0.0f);
}

void RigidBodyComponent::SetPosition(const glm::vec3& position) {
    if (m_RigidBody) {
        m_RigidBody->SetPosition(position);
    }
}

std::shared_ptr<RigidBody> RigidBodyComponent::GetRigidBody() const {
    return m_RigidBody;
}
