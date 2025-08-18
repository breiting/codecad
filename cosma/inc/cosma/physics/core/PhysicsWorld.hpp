#pragma once
#include <physics/core/RigidBody.hpp>
#include <vector>

class PhysicsWorld {
   public:
    void AddRigidBody(std::shared_ptr<RigidBody> body);
    void Update(float dt);
    void SetGlobalGravity(const glm::vec3& gravity);

   private:
    glm::vec3 m_Gravity = glm::vec3(0.0f, 0.0f, -9.81f);
    std::vector<std::shared_ptr<RigidBody>> m_Bodies;
};
