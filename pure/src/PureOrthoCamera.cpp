#include "pure/PureOrthoCamera.hpp"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

static inline float deg2rad(float d) {
    return d * 0.017453292519943295769f;
}  // pi/180

PureOrthoCamera::PureOrthoCamera(float radius, float pitchDeg, float yawDeg)
    : m_RotationDeg{yawDeg, pitchDeg}, m_Radius(radius) {
    UpdatePosition();
}

void PureOrthoCamera::SetAspect(float aspect) {
    m_Aspect = std::max(0.01f, aspect);
}

void PureOrthoCamera::FitToBounds(const PureBounds& bounds, float padding) {
    if (!bounds.valid) return;
    m_Bounds = bounds;

    const glm::vec3 size = bounds.Size();
    const glm::vec3 center = bounds.Center();
    m_Target = center;

    // Orthographic fit: choose half-height to cover both axes with padding
    const float halfH = 0.5f * size.y;
    const float halfW = 0.5f * size.x;
    const float neededHalfH = std::max(halfH, halfW / std::max(0.01f, m_Aspect));
    m_OrthoHalfHeight = std::clamp(neededHalfH * padding, m_OrthoMin, m_OrthoMax);

    // Keep a friendly viewing angle
    m_RotationDeg = {35.0f, 22.0f};
    ClampPitch();

    UpdateClipPlanes(std::max({size.x, size.y, size.z, 1.0f}));
    UpdatePosition();
}

void PureOrthoCamera::OnScrollWheel(float delta) {
    // delta>0 => zoom in; use exponential scaling
    const float step = std::clamp((float)delta, -5.0f, 5.0f);
    const float factor = std::pow(1.0f / m_ZoomSpeed, step);
    m_OrthoHalfHeight = std::clamp(m_OrthoHalfHeight * factor, m_OrthoMin, m_OrthoMax);
}

void PureOrthoCamera::Orbit(float dx, float dy) {
    // Mouse deltas in pixels → degrees
    m_RotationDeg.x += (float)dx * m_OrbitSpeed;  // yaw
    m_RotationDeg.y += (float)dy * m_OrbitSpeed;  // pitch
    ClampPitch();
    UpdatePosition();
}

void PureOrthoCamera::Pan(float dx, float dy) {
    // Pan along image plane; scale by radius and current ortho size
    // Convert pixels to world: use ortho window extent as scale.
    float hh = std::max(m_OrthoMin, m_OrthoHalfHeight);
    float hw = hh * std::max(0.01f, m_Aspect);

    // Normalize pixel deltas by a typical screen size (~ height), then scale into world
    // Feel free to replace 800.0f by your actual framebuffer height.
    const float pxNorm = 1.0f / 800.0f;
    const float sx = 2.0f * hw * pxNorm;  // world units per pixel horizontally
    const float sy = 2.0f * hh * pxNorm;  // world units per pixel vertically

    // Build camera basis
    glm::vec3 f = ViewDirection();
    glm::vec3 r = glm::normalize(glm::cross(f, glm::vec3(0, 1, 0)));
    glm::vec3 u = glm::normalize(glm::cross(r, f));

    m_Target -= r * ((float)dx * sx * m_PanScale);
    m_Target += u * ((float)dy * sy * m_PanScale);
    UpdatePosition();
}

glm::mat4 PureOrthoCamera::View() const {
    return glm::lookAt(m_Position, m_Target, glm::vec3(0, 1, 0));
}

glm::mat4 PureOrthoCamera::Projection(float near, float far) const {
    float hh = std::max(m_OrthoMin, m_OrthoHalfHeight);
    float hw = hh * std::max(0.01f, m_Aspect);
    return glm::ortho(-hw, +hw, -hh, +hh, std::max(0.0001f, near), std::max(near + 1e-4f, far));
}

glm::vec3 PureOrthoCamera::ViewDirection() const {
    return glm::normalize(m_Target - m_Position);
}

void PureOrthoCamera::UpdatePosition() {
    // Orbit math identical to perspective: eye on sphere around target
    const float rp = deg2rad(m_RotationDeg.y);
    const float ry = deg2rad(m_RotationDeg.x);

    glm::vec3 offset;
    offset.x = m_Radius * std::cos(rp) * std::cos(ry);
    offset.y = m_Radius * std::sin(rp);
    offset.z = m_Radius * std::cos(rp) * std::sin(ry);

    m_Position = m_Target + offset;
}

void PureOrthoCamera::UpdateClipPlanes(float sceneScale) {
    // For ortho, near/far just need to bracket the scene comfortably.
    // const float nearMin = 0.001f * sceneScale;
    // m_Near = std::max(nearMin, 0.05f * sceneScale);
    // m_Far = 6.0f * sceneScale + m_Near;
    // if (m_Far <= m_Near + 1e-4f) m_Far = m_Near + 1.0f;
}

void PureOrthoCamera::ClampPitch() {
    // avoid flipping at poles
    m_RotationDeg.y = std::clamp(m_RotationDeg.y, -89.0f, 89.0f);
}
