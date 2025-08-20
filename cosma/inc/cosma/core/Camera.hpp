#pragma once

#include <glm/glm.hpp>

// Abstract class
class Camera {
   public:
    virtual glm::mat4 GetViewMatrix() const = 0;
    virtual void SetAspectRatio(float aspect) = 0;
    virtual glm::mat4 GetProjectionMatrix() const = 0;

    // Handle mouse interaction
    virtual void OnMouseStart() = 0;
    virtual void OnMouseRotation(double xpos, double ypos) = 0;
    virtual void OnMousePan(double xpos, double ypos) = 0;
    virtual void OnMouseScroll(double yoffset) = 0;

    virtual void Update(float deltaTime) = 0;

    virtual glm::vec3 GetViewDirection() const = 0;

    virtual void SetPosition(const glm::vec3& position) = 0;
    virtual void SetTarget(const glm::vec3& target) = 0;
    virtual const glm::vec3& GetPosition() const = 0;
    virtual const glm::vec3& GetTarget() const = 0;

    virtual ~Camera() = default;
};
