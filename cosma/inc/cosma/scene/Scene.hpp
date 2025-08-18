#pragma once

#include <core/DirectionalLight.hpp>
#include <memory>
#include <scene/SceneNode.hpp>
#include <vector>

class Scene {
   public:
    void AddNode(std::shared_ptr<SceneNode> node);
    void SetLightDirectionalLight(std::shared_ptr<DirectionalLight> light);

    void Update(float deltaTime);
    void Render(const glm::mat4& view, const glm::mat4& projection);

    std::shared_ptr<DirectionalLight> GetDirectionalLight();

   private:
    std::vector<std::shared_ptr<SceneNode>> m_Nodes;
    std::shared_ptr<DirectionalLight> m_DirectionalLight;
};
