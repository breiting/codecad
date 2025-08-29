#pragma once
#include <glm/glm.hpp>

#include "PureScene.hpp"

namespace pure {

class PureMesh;
class PureShader;

struct PureRenderParams {
    glm::vec3 baseColor{0.85f, 0.82f, 0.78f};
    glm::vec3 lightDir = glm::normalize(glm::vec3(-1.0f, -1.0f, -0.5f));
};

void DrawMesh(const PureMesh& mesh, const PureShader& shader, const glm::mat4& model, const glm::mat4& view,
              const glm::mat4& proj, const glm::vec3& camPos, const PureRenderParams& params);

void DrawScene(std::shared_ptr<PureScene> scene, const PureShader& shader, const glm::mat4& view, const glm::mat4& proj,
               const glm::vec3& camPos, const glm::vec3& camViewDir /* Headlight */);

}  // namespace pure
