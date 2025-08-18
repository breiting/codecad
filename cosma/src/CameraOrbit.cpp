#include <algorithm>
#include <core/CameraOrbit.hpp>
#include <glm/ext/matrix_clip_space.hpp>  // glm::perspective
#include <glm/ext/matrix_transform.hpp>   // glm::lookat
#include <glm/glm.hpp>

CameraOrbit::CameraOrbit(float radius, float pitch, float yaw)
    : m_Radius(radius),
      m_Rotation({yaw, pitch}),
      m_RotationVelocity(0.0f),
      m_FirstMouse(true),
      m_LastMouse(0.0f),
      m_DampingFactor(0.90f),
      m_VelocityThreshold(0.001f) {
    UpdatePosition();
}

void CameraOrbit::OnMouseStart() {
    m_FirstMouse = true;
}

glm::vec2 CameraOrbit::OnMouseMove(double xpos, double ypos) {
    if (m_FirstMouse) {
        m_LastMouse = glm::vec2(xpos, ypos);
        m_FirstMouse = false;
    }

    glm::vec2 currentMouse(xpos, ypos);
    glm::vec2 delta = currentMouse - m_LastMouse;
    m_LastMouse = currentMouse;

    float sensitivity = 2.0f;
    delta *= sensitivity;
    return delta;
}

void CameraOrbit::OnMouseRotation(double xpos, double ypos) {
    auto delta = OnMouseMove(xpos, ypos);
    float speed = 1.0f;
    m_RotationVelocity += delta * speed;
    UpdatePosition();
}

void CameraOrbit::OnMousePan(double xpos, double ypos) {
    auto delta = OnMouseMove(xpos, ypos);
    glm::vec3 right = glm::normalize(glm::cross(GetViewDirection(), glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up = glm::normalize(glm::cross(right, GetViewDirection()));

    m_Target -= right * delta.x * 0.001f;
    m_Target += up * delta.y * 0.001f;
    UpdatePosition();
}

void CameraOrbit::OnMouseScroll(double yoffset) {
    float factor = static_cast<float>(yoffset);
    float zoomSpeed = 0.2f;
    factor = copysignf(powf(std::abs(factor), 1.2f), factor);  // nichtlinear
    m_Radius -= factor * zoomSpeed;
    m_Radius = std::clamp(m_Radius, 1.0f, 100.0f);
    UpdatePosition();
}

void CameraOrbit::Update(float deltaTime) {
    m_Rotation += m_RotationVelocity * deltaTime;
    m_Rotation.y = std::clamp(m_Rotation.y, -89.0f, 89.0f);
    m_RotationVelocity *= m_DampingFactor;
    if (glm::length(m_RotationVelocity) < m_VelocityThreshold) {
        m_RotationVelocity = glm::vec2(0.0f);
    }
    UpdatePosition();
}

void CameraOrbit::UpdatePosition() {
    float radPitch = glm::radians(m_Rotation.y);
    float radYaw = glm::radians(m_Rotation.x);

    glm::vec3 offset;
    offset.x = m_Radius * cos(radPitch) * cos(radYaw);
    offset.y = m_Radius * sin(radPitch);
    offset.z = m_Radius * cos(radPitch) * sin(radYaw);

    m_Position = m_Target + offset;
}

void CameraOrbit::SetPosition(const glm::vec3& position) {
    m_Position = position;
}

void CameraOrbit::SetTarget(const glm::vec3& target) {
    m_Target = target;
}

const glm::vec3& CameraOrbit::GetPosition() const {
    return m_Position;
}

const glm::vec3& CameraOrbit::GetTarget() const {
    return m_Target;
}

glm::mat4 CameraOrbit::GetViewMatrix() const {
    auto up = glm::vec3(0.0f, 1.0f, 0.0f);
    return glm::lookAt(m_Position, m_Target, up);
}

glm::mat4 CameraOrbit::GetProjectionMatrix() const {
    // TODO: FOV should be dynamic
    float fov = 45.0;
    return glm::perspective(glm::radians(fov), m_AspectRatio, 0.01f, 500.0f);
}

void CameraOrbit::SetAspectRatio(float aspect) {
    m_AspectRatio = aspect;
}

glm::vec3 CameraOrbit::GetViewDirection() const {
    return glm::normalize(m_Target - m_Position);
}
