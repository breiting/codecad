#include <algorithm>
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

void Scene::RemoveNode(std::shared_ptr<SceneNode> node) {
    if (!node) return;
    SceneNode* ptr = node.get();
    auto it = std::remove_if(m_Nodes.begin(), m_Nodes.end(),
                             [ptr](const std::shared_ptr<SceneNode>& n) { return n.get() == ptr; });
    m_Nodes.erase(it, m_Nodes.end());
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

AABB Scene::ComputeWorldBounds() const {
    AABB out;
    for (const auto& n : m_Nodes) {
        if (!n) continue;
        out.Include(n->GetWorldAABB());
    }
    return out;
}
