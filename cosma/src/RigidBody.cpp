#include <physics/core/RigidBody.hpp>

RigidBody::RigidBody()
    : m_Position(0.0f), m_Velocity(0.0f), m_Acceleration(0.0f), m_Mass(1.0f), m_Restitution(0.8f), m_Radius(1.0f) {
}

void RigidBody::ApplyForce(const glm::vec3& force) {
    m_Acceleration += force / m_Mass;
}

void RigidBody::Reset() {
    m_Velocity = glm::vec3(0.0f);
    m_Acceleration = glm::vec3(0.0f);
}

void RigidBody::Update(float dt) {
    m_Velocity += m_Acceleration * dt;
    m_Position += m_Velocity * dt;

    CheckCollisionWithGround();
    m_Acceleration = glm::vec3(0.0f);
}

void RigidBody::CheckCollisionWithGround() {
    // Check for collision with the ground (z = 0)
    if (m_Position.z - m_Radius <= 0.0f) {
        m_Position.z = m_Radius;  // Set position to the ground level

        // Rebound effect: reverse velocity in the Z direction
        if (m_Velocity.z < 0.0f) {
            m_Velocity.z *= -m_Restitution;  // Elastic rebound
        }

        // Apply friction on the XY plane to slow down horizontal velocity
        glm::vec3 horizontalVelocity = glm::vec3(m_Velocity.x, m_Velocity.y, 0.0f);
        float frictionFactor = 0.98f;  // Friction factor (adjustable)

        // Gradually reduce the horizontal velocity by friction
        if (glm::length(horizontalVelocity) > 0.1f) {
            m_Velocity.x *= frictionFactor;
            m_Velocity.y *= frictionFactor;
        } else {
            // Stop the ball if the velocity is very low
            m_Velocity.x = 0.0f;
            m_Velocity.y = 0.0f;
        }
    }
}

void RigidBody::SetPosition(const glm::vec3& pos) {
    m_Position = pos;
}

const glm::vec3& RigidBody::GetPosition() const {
    return m_Position;
}

void RigidBody::SetVelocity(const glm::vec3& vel) {
    m_Velocity = vel;
}

const glm::vec3& RigidBody::GetVelocity() const {
    return m_Velocity;
}

void RigidBody::SetMass(float mass) {
    m_Mass = mass;
}

const float RigidBody::GetMass() const {
    return m_Mass;
}

void RigidBody::SetAcceleration(const glm::vec3& acc) {
    m_Acceleration = acc;
}

const glm::vec3& RigidBody::GetAcceleration() const {
    return m_Acceleration;
}

void RigidBody::SetRestitution(float rest) {
    m_Restitution = rest;
}

const float RigidBody::GetRestitution() const {
    return m_Restitution;
}

void RigidBody::SetRadius(float rad) {
    m_Radius = rad;
}
const float RigidBody::GetRadius() const {
    return m_Radius;
}

void RigidBody::Shoot(float angleDegrees, float initialVelocity) {
    // TODO: just an experiment currently
    float angleRadians = glm::radians(angleDegrees);

    // Zerlegung der Anfangsgeschwindigkeit in Komponenten (X und Z)
    glm::vec3 velocity;
    velocity.x = initialVelocity * glm::cos(angleRadians);  // Horizontale Geschwindigkeit
    velocity.z = initialVelocity * glm::sin(angleRadians);  // Vertikale Geschwindigkeit

    // Setze die berechnete Anfangsgeschwindigkeit
    SetVelocity(velocity);
}
