#pragma once

#include <core/Camera.hpp>

class CameraOrbit : public Camera {
   public:
    CameraOrbit(float radius = 5.0f, float pitch = 0.0f, float yaw = 0.0f);

    void OnMouseStart() override;
    void OnMouseRotation(double xpos, double ypos) override;
    void OnMousePan(double xpos, double ypos) override;
    void OnMouseScroll(double yoffset) override;

    void Update(float deltaTime) override;

    glm::mat4 GetViewMatrix() const override;
    glm::mat4 GetProjectionMatrix() const override;
    void SetAspectRatio(float aspect) override;
    glm::vec3 GetViewDirection() const override;

    void SetPosition(const glm::vec3& position) override;
    void SetTarget(const glm::vec3& target) override;
    const glm::vec3& GetPosition() const override;
    const glm::vec3& GetTarget() const override;

   private:
    void UpdatePosition();
    glm::vec2 OnMouseMove(double xpos, double ypos);

   private:
    float m_AspectRatio;

    glm::vec3 m_Position;
    glm::vec2 m_Rotation;          // yaw, pitch
    glm::vec2 m_RotationVelocity;  // rad, sec

    glm::vec3 m_Target = glm::vec3(0.0f);       // LookAt target point
    glm::vec2 m_PanVelocity = glm::vec2(0.0f);  // Move velocity for panning

    float m_Radius;
    float m_DampingFactor;
    float m_VelocityThreshold;

    glm::vec2 m_LastMouse;
    bool m_FirstMouse;
};
