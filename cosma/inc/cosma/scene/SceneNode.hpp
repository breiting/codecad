#pragma once

#include <core/Transform.hpp>
#include <scene/Renderable.hpp>

class SceneNode {
   public:
    SceneNode(const std::string& name = "");
    virtual ~SceneNode() = default;

    void SetPosition(const glm::vec3& position);
    void SetScale(const glm::vec3& scale);
    void SetRotationEuler(const glm::vec3& eulerAngles);
    void SetRotation(const glm::quat& rotation);

    void SetName(const std::string& name);
    std::string GetName() const;

    void Move(const glm::vec3& delta);

    virtual void Update(float deltaTime) = 0;
    virtual void Render(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                        std::shared_ptr<Light> light) = 0;

   protected:
    Transform m_Transform;
    std::string m_Name;
};
