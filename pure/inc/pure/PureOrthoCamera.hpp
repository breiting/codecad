#pragma once
#include <glm/glm.hpp>

#include "pure/IPureCamera.hpp"

/**
 * @brief Orthographic orbit camera.
 *
 * Conventions:
 * - World up is +Y.
 * - Orbit is defined by yaw/pitch around a target point.
 * - Zoom changes the orthographic half-height (window size), independent of distance.
 */
class PureOrthoCamera : public IPureCamera {
   public:
    /// Construct with initial radius (orbit distance), initial pitch/yaw (deg).
    explicit PureOrthoCamera(float radius = 3.0f, float pitchDeg = 22.0f, float yawDeg = 35.0f);

    // IPureCamera
    void SetAspect(float aspect) override;
    void FitToBounds(const PureBounds& bounds, float padding = 1.1f) override;
    void OnScrollWheel(float delta) override;
    void Orbit(float dx, float dy) override;
    void Pan(float dx, float dy) override;
    glm::mat4 View() const override;
    glm::mat4 Projection(float near, float far) const override;
    glm::vec3 Position() const override {
        return m_Position;
    }
    glm::vec3 ViewDirection() const override;

    // Optional tuning knobs
    void SetZoomSpeed(float s) {
        m_ZoomSpeed = s;
    }
    void SetPanScale(float s) {
        m_PanScale = s;
    }
    void SetOrbitSpeed(float s) {
        m_OrbitSpeed = s;
    }
    void SetOrthoRange(float minHalfH, float maxHalfH) {
        m_OrthoMin = minHalfH;
        m_OrthoMax = maxHalfH;
    }

    void SetBounds(const PureBounds& b) {
        m_Bounds = b;
    }

   private:
    void UpdatePosition();
    void UpdateClipPlanes(float sceneScale);
    void ClampPitch();

   private:
    // Orbit state
    glm::vec3 m_Target{0.0f};
    glm::vec2 m_RotationDeg{35.0f, 22.0f};  // yaw (x), pitch (y) in degrees
    float m_Radius = 3.0f;                  // distance eye ↔ target (used only for orbit math)

    // Projection
    float m_Aspect = 1.6f;

    // Orthographic window sizing
    float m_OrthoHalfHeight = 1.0f;  // controls zoom
    float m_OrthoMin = 1e-3f, m_OrthoMax = 1e6f;

    // Speeds
    float m_ZoomSpeed = 1.15f;   // > 1.0 (scroll zoom factor base)
    float m_PanScale = 0.002f;   // pan world-units per pixel (scaled by radius)
    float m_OrbitSpeed = 0.25f;  // deg per pixel

    // Cached
    glm::vec3 m_Position{0.0f};
    PureBounds m_Bounds{};
};
