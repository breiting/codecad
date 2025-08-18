#include <glm/trigonometric.hpp>
#include <scene/ShapeNode.hpp>

ShapeNode::ShapeNode() {
}

void ShapeNode::SetMaterial(std::shared_ptr<Material> material) {
    if (m_Renderable) {
        m_Renderable->material = material;
    }
}

void ShapeNode::Update(float deltaTime) {
}

void ShapeNode::Render(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                       std::shared_ptr<Light> light) {
    glm::mat4 worldTransform = model * m_Transform.GetTransformationMatrix();

    if (m_Renderable) {
        m_Renderable->Render(worldTransform, view, projection, light);
    }
}
