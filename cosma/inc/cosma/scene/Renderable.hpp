#pragma once
#include <core/Transform.hpp>
#include <geometry/LineSet.hpp>
#include <geometry/Mesh.hpp>
#include <material/Material.hpp>

class Renderable {
   public:
    std::unique_ptr<Mesh> mesh;
    std::unique_ptr<LineSet> lines;
    std::shared_ptr<Material> material;

    void Render(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                std::shared_ptr<Light> light);
};
