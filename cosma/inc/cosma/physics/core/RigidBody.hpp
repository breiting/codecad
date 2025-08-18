#pragma once
#include <glm/glm.hpp>

class RigidBody {
   public:
    RigidBody();
    void ApplyForce(const glm::vec3& force);
    void Update(float dt);

    void SetPosition(const glm::vec3& pos);
    const glm::vec3& GetPosition() const;

    void SetVelocity(const glm::vec3& vel);
    const glm::vec3& GetVelocity() const;

    void SetMass(float mass);
    const float GetMass() const;

    void SetAcceleration(const glm::vec3& acc);
    const glm::vec3& GetAcceleration() const;

    void SetRestitution(float rest);
    const float GetRestitution() const;

    void SetRadius(float rad);
    const float GetRadius() const;

    void Reset();

    void CheckCollisionWithGround();

    void Shoot(float angleDegrees, float initialVelocity);

   private:
    glm::vec3 m_Position = glm::vec3(0.0f);
    glm::vec3 m_Velocity = glm::vec3(0.0f);
    float m_Mass = 1.0f;
    glm::vec3 m_Acceleration = glm::vec3(0.0f);
    float m_Restitution = 0.8f;  // preserving energy after bounce
    float m_Radius;
};
