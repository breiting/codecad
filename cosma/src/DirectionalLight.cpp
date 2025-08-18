#include <core/DirectionalLight.hpp>

glm::vec3 DirectionalLight::GetColor() const {
    return m_Color;
}

glm::vec3 DirectionalLight::GetDirection() const {
    return m_Direction;
}

glm::vec3 DirectionalLight::GetPosition() const {
    return glm::vec3(0.0f, 0.0f, 0.0f);
}

void DirectionalLight::SetColor(const glm::vec3 &color) {
    m_Color = color;
}

void DirectionalLight::SetDirection(const glm::vec3 &dir) {
    m_Direction = dir;
}
