#pragma once
#include <material/Material.hpp>
#include <memory>

class LineSetMaterial : public Material {
   public:
    explicit LineSetMaterial();

    void Apply(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
               std::shared_ptr<Light> light) override;
    std::shared_ptr<Shader> GetShader() const override;

   private:
    std::shared_ptr<Shader> m_Shader;
};
