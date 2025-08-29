#pragma once

#include <core/Transform.hpp>
#include <scene/Renderable.hpp>
#include <scene/SceneNode.hpp>

class ShapeNode : public SceneNode {
   public:
    ShapeNode();
    virtual ~ShapeNode() = default;

    void SetMaterial(std::shared_ptr<Material> material);

    void Update(float deltaTime) override;
    void Render(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                std::shared_ptr<Light> light) override;

    AABB GetWorldAABB() const override;

   protected:
    std::shared_ptr<Renderable> m_Renderable;
};
