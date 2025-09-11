#pragma once
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include "pure/IPureCamera.hpp"

namespace pure {
class PurePerspectiveCamera : public IPureCamera {
   public:
    PurePerspectiveCamera();

    void SetAspect(float aspect) override;
    void SetFovDeg(float degrees) override;
    void SetTarget(const glm::vec3& target) override;
    void SetPosition(const glm::vec3& pos) override;
    void SetRadius(float radius);

    // Mouse control
    void Orbit(float deltaX, float deltaY) override;
    void Pan(float deltaX, float deltaY) override;

    void OnScrollWheel(float yoff) override;

    // BBox-Utilities
    void FitToBounds(const PureBounds& bounds, float padding = 1.1f) override;

    glm::vec3 Target() const override {
        return m_Target;
    }
    float FovDeg() const override {
        return m_FovDeg;
    }

    // Matrices
    glm::mat4 View() const override;  // Z-up
    glm::mat4 Projection() const override;

    // Getter
    glm::vec3 Position() const override {
        return m_Position;
    }
    glm::vec3 ViewDirection() const override {
        return glm::normalize(m_Target - m_Position);
    }
    float Aspect() const {
        return m_Aspect;
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

    float m_Near = 0.01f;
    float m_Far = 1000.0f;

    // Maus-Skalierung
    float m_YawScale = -0.2f;  // invert for left-drag = left=rotation
    float m_PitchScale = 0.2f;
    float m_PanScale = 0.002f;
    float m_ZoomImpulse = 0.08f;

    glm::vec3 m_Target{0.0f, 0.0f, 0.0f};
    glm::vec3 m_Position{0.0f, 0.0f, 5.0f};
};
}  // namespace pure
