#pragma once
#include <material/Material.hpp>
#include <memory>

class InfinityGridMaterial : public Material {
   public:
    explicit InfinityGridMaterial();

    void Apply(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
               std::shared_ptr<Light> light) override;
    std::shared_ptr<Shader> GetShader() const override;

   private:
    unsigned int m_Vao;
    std::shared_ptr<Shader> m_Shader;
};
