#include <glm/gtc/matrix_inverse.hpp>
#include <pure/PureMesh.hpp>
#include <pure/PureRenderer.hpp>
#include <pure/PureScene.hpp>
#include <pure/PureShader.hpp>

namespace pure {

void PureRenderer::DrawMesh(const PureMesh& mesh, const PureShader& shader, const glm::mat4& model,
                            const glm::mat4& view, const glm::mat4& proj, const glm::vec3& camPos,
                            const PureRenderParams& params) {
    shader.Use();
    shader.SetMat4("u_model", model);
    shader.SetMat4("u_view", view);
    shader.SetMat4("u_proj", proj);
    shader.SetMat3("u_normalMatrix", glm::mat3(glm::transpose(glm::inverse(model))));
    shader.SetVec3("u_camPos", camPos);
    shader.SetVec3("u_lightDir", glm::normalize(params.lightDir));
    shader.SetVec3("u_baseColor", params.baseColor);
    mesh.Draw();
}

void PureRenderer::DrawScene(std::shared_ptr<PureScene> scene, const PureShader& shader, const glm::mat4& view,
                             const glm::mat4& proj, const glm::vec3& camPos, const glm::vec3& camViewDir) {
    glPolygonMode(GL_FRONT_AND_BACK, m_Wireframe ? GL_LINE : GL_FILL);
    // Headlight = Camera direction
    for (const auto& part : scene->Parts()) {
        PureRenderParams rp;
        rp.baseColor = part.material.baseColor;
        rp.lightDir = camViewDir;
        DrawMesh(*part.mesh, shader, part.model, view, proj, camPos, rp);
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

}  // namespace pure
