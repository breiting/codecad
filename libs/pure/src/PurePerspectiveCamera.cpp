#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <pure/PurePerspectiveCamera.hpp>

namespace pure {

PurePerspectiveCamera::PurePerspectiveCamera() {
    UpdatePosition();
}

void PurePerspectiveCamera::SetAspect(float aspect) {
    m_Aspect = (aspect > 0.f ? aspect : 1.f);
}
void PurePerspectiveCamera::SetFovDeg(float degrees) {
    m_FovDeg = degrees;
}
void PurePerspectiveCamera::SetTarget(const glm::vec3& target) {
    m_Target = target;
    UpdatePosition();
}
void PurePerspectiveCamera::SetPosition(const glm::vec3& pos) {
    m_Position = pos;
    UpdatePosition();
}
void PurePerspectiveCamera::SetRadius(float radius) {
    m_Radius = (radius > 1e-6f ? radius : 1.f);
    UpdatePosition();
}

void PurePerspectiveCamera::Orbit(float deltaX, float deltaY) {
    m_Yaw += deltaX * m_YawScale;
    m_Pitch += deltaY * m_PitchScale;
    m_Pitch = std::clamp(m_Pitch, -89.0f, 89.0f);
    UpdatePosition();
}

void PurePerspectiveCamera::Pan(float deltaX, float deltaY) {
    float scale = m_Radius * m_PanScale;
    glm::vec3 forward = glm::normalize(ViewDirection());
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 0, 1)));
    glm::vec3 up = glm::normalize(glm::cross(right, forward));
    m_Target -= right * (deltaX * scale);
    m_Target += up * (deltaY * scale);
    UpdatePosition();
}

void PurePerspectiveCamera::FitToBounds(const PureBounds& bounds, float padding) {
    glm::vec3 center = 0.5f * (bounds.min + bounds.max);
    glm::vec3 size = (bounds.max - bounds.min);
    float rSphere = 0.5f * glm::length(size);
    float vfov = glm::radians(m_FovDeg);
    float hfov = 2.0f * std::atan(std::tan(vfov * 0.5f) * m_Aspect);
    float dist = padding * std::max(rSphere / std::sin(vfov * 0.5f), rSphere / std::sin(hfov * 0.5f));
    if (!std::isfinite(dist) || dist < 1e-3f) dist = 1.0f;
    m_Target = center;
    m_Radius = dist;
    UpdatePosition();
}

glm::mat4 PurePerspectiveCamera::View() const {
    // Z-up!
    return glm::lookAt(m_Position, m_Target, glm::vec3(0, 0, 1));
}

glm::mat4 PurePerspectiveCamera::Projection() const {
    return glm::perspective(glm::radians(m_FovDeg), m_Aspect, m_Near, m_Far);
}

void PurePerspectiveCamera::OnScrollWheel(float yoff) {
    const float sign = (yoff > 0.0 ? -1.0f : (yoff < 0.0 ? +1.0f : 0.0f));
    if (sign != 0.0f) {
        const float factor = std::exp(m_ZoomImpulse * sign * std::abs((float)yoff));
        m_Radius = std::clamp(m_Radius * factor, 0.001f, 1e6f);
        UpdatePosition();
    }
}

void PurePerspectiveCamera::UpdatePosition() {
    float pitchRad = glm::radians(m_Pitch);
    float yawRad = glm::radians(m_Yaw);

    glm::vec3 offset;
    offset.x = m_Radius * std::cos(pitchRad) * std::cos(yawRad);
    offset.y = m_Radius * std::cos(pitchRad) * std::sin(yawRad);
    offset.z = m_Radius * std::sin(pitchRad);

    m_Position = m_Target + offset;
}
}  // namespace pure
