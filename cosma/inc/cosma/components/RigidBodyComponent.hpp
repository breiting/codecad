#pragma once

#include <components/PhysicsComponent.hpp>
#include <glm/glm.hpp>
#include <memory>

class RigidBody;
class PhysicsWorld;

class RigidBodyComponent : public PhysicsComponent {
   public:
    RigidBodyComponent(std::shared_ptr<RigidBody> rigidBody);

    void Update(float deltaTime) override;
    void ApplyToPhysicsWorld(PhysicsWorld& world) override;

    std::shared_ptr<RigidBody> GetRigidBody() const;
    glm::vec3 GetPosition() const;
    void SetPosition(const glm::vec3& position);

   private:
    std::shared_ptr<RigidBody> m_RigidBody;
};
