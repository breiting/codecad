#pragma once
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

namespace pure {
class PureCamera {
   public:
    PureCamera();

    void SetAspect(float aspect);
    void SetFovDegrees(float degrees);
    void SetTarget(const glm::vec3& target);
    void SetRadius(float radius);

    // Mouse control
    void Orbit(float deltaX, float deltaY);  // Pixel
    void Pan(float deltaX, float deltaY);    // Pixel
    void Dolly(float wheelSteps);            // Mouse wheel steps

    void OnScrollWheel(double yoff);

    // BBox-Utilities
    void FitToBounds(const glm::vec3& boundsMin, const glm::vec3& boundsMax, float padding = 1.12f);
    void FocusBounds(const glm::vec3& boundsMin, const glm::vec3& boundsMax);

    // Matrices
    glm::mat4 View() const;  // Z-up
    glm::mat4 Projection(float nearZ, float farZ) const;

    // Getter
    glm::vec3 Position() const {
        return m_Position;
    }
    glm::vec3 Target() const {
        return m_Target;
    }
    glm::vec3 ViewDirection() const {
        return glm::normalize(m_Target - m_Position);
    }
    float Aspect() const {
        return m_Aspect;
    }
    float FovDegrees() const {
        return m_FovDeg;
    }
    float Radius() const {
        return m_Radius;
    }

    // Controls
    void SetYawScale(float s) {
        m_YawScale = s;
    }  // default -0.2 (left drag means left rotation)
    void SetPitchScale(float s) {
        m_PitchScale = s;
    }  // default 0.2
    void SetPanScale(float s) {
        m_PanScale = s;
    }  // default 0.002

   private:
    void UpdatePosition();

   private:
    // Grad
    float m_Yaw = 35.0f;    // around Z
    float m_Pitch = 22.0f;  // Up/down

    float m_Radius = 3.0f;
    float m_Aspect = 16.0f / 9.0f;
    float m_FovDeg = 45.0f;

    // Maus-Skalierung
    float m_YawScale = -0.2f;  // invert for left-drag = left=rotation
    float m_PitchScale = 0.2f;
    float m_PanScale = 0.002f;
    float m_ZoomImpulse = 0.08f;

    glm::vec3 m_Target{0.0f, 0.0f, 0.0f};
    glm::vec3 m_Position{0.0f, 0.0f, 5.0f};
};
}  // namespace pure
