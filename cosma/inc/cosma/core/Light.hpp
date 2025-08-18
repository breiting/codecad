#pragma once

#include <glm/glm.hpp>
class Light {
   public:
    virtual ~Light() = default;

    virtual glm::vec3 GetColor() const = 0;
    virtual glm::vec3 GetDirection() const = 0;
    virtual glm::vec3 GetPosition() const = 0;
};
