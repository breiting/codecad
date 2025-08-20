#include <scene/Renderable.hpp>

void Renderable::Render(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                        std::shared_ptr<Light> light) {
    material->Apply(model, view, projection, light);
    if (mesh) {
        mesh->Upload();
        mesh->Render();
    }
    if (lines) {
        lines->Upload();
        lines->Render();
    }
}
