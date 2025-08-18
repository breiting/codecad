#include <physics/core/PhysicsWorld.hpp>

void PhysicsWorld::AddRigidBody(std::shared_ptr<RigidBody> body) {
    m_Bodies.push_back(body);
}

void PhysicsWorld::SetGlobalGravity(const glm::vec3& gravity) {
    m_Gravity = gravity;
}

void PhysicsWorld::Update(float dt) {
    for (auto body : m_Bodies) {
        body->ApplyForce(m_Gravity * body->GetMass());  // F = m * g
        body->Update(dt);
    }
}
