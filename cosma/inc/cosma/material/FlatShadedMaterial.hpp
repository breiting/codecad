#pragma once
#include <core/Texture.hpp>
#include <material/Material.hpp>
#include <memory>

class FlatShadedMaterial : public Material {
   public:
    explicit FlatShadedMaterial(glm::vec3 materialColor = {0.1f, 0.1f, 0.1f});

    void Apply(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
               std::shared_ptr<Light> light) override;

    std::shared_ptr<Shader> GetShader() const override;
    void SetLightDirection(const glm::vec3& dir);
    void SetLightColor(const glm::vec3& color);
    void SetMaterialColor(const glm::vec3& color);
    void SetTexture(std::shared_ptr<Texture> texture);

   private:
    std::shared_ptr<Shader> m_Shader;
    glm::vec3 m_MaterialColor;
    glm::vec3 m_LightDirection;
    glm::vec3 m_LightColor;
    std::shared_ptr<Texture> m_Texture;
    unsigned int m_TextureLocation;
};
