#pragma once

#include <core/Transform.hpp>
#include <scene/Renderable.hpp>
#include <scene/SceneNode.hpp>

#include "components/RigidBodyComponent.hpp"

class RigidBodyComponent;

class ShapeNode : public SceneNode {
   public:
    ShapeNode();
    virtual ~ShapeNode() = default;

    void SetMaterial(std::shared_ptr<Material> material);

    void Update(float deltaTime) override;
    void Render(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                std::shared_ptr<Light> light) override;

    void AttachRigidBody(std::shared_ptr<RigidBodyComponent> rigidBody);
    bool HasPhysics() const;

   protected:
    std::shared_ptr<Renderable> m_Renderable;
    std::shared_ptr<RigidBodyComponent> m_RigidBody;
};
