#pragma once
#include <glm/glm.hpp>
#include <memory>

class Shader;
class Light;

class Material {
   public:
    virtual ~Material() = default;

    virtual void Apply(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                       std::shared_ptr<Light> light) = 0;

    virtual std::shared_ptr<Shader> GetShader() const = 0;
};
