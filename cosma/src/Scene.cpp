#include <core/DirectionalLight.hpp>
#include <memory>
#include <scene/Scene.hpp>
#include <vector>

/**
 * Make sure to convert all geometry into our coordinate system where +z is the height. Therefore we just flip Y and Z
 */
const glm::mat4 GLOBAL_WORLD_TRANSFORM = glm::mat4(1, 0, 0, 0,   //
                                                   0, 0, -1, 0,  //
                                                   0, 1, 0, 0,   //
                                                   0, 0, 0, 1);

void Scene::AddNode(std::shared_ptr<SceneNode> node) {
    m_Nodes.push_back(std::move(node));
}

void Scene::SetLightDirectionalLight(std::shared_ptr<DirectionalLight> light) {
    m_DirectionalLight = light;
}

std::shared_ptr<DirectionalLight> Scene::GetDirectionalLight() {
    return m_DirectionalLight;
}

void Scene::Update(float deltaTime) {
    for (auto& node : m_Nodes) {
        node->Update(deltaTime);
    }
}

void Scene::Render(const glm::mat4& view, const glm::mat4& projection) {
    for (auto& node : m_Nodes) {
        node->Render(GLOBAL_WORLD_TRANSFORM, view, projection, m_DirectionalLight);
    }
}
