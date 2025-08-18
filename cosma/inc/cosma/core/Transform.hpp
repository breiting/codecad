#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform {
   public:
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::quat rotation{glm::quat(1.0f, 0.0f, 0.0f, 0.0f)};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    glm::mat4 GetTransformationMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model *= glm::mat4_cast(rotation);
        model = glm::scale(model, scale);
        return model;
    }
};
