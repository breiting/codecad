// clang-format off
#include <glad.h>
// clang-format on
#include <material/InfinityGridMaterial.hpp>
#include <material/Shader.hpp>
#include <string>

#include "assets/grid_frag.h"
#include "assets/grid_vert.h"

InfinityGridMaterial::InfinityGridMaterial() {
    m_Shader = std::make_shared<Shader>(grid_vert_glsl, grid_frag_glsl);
    glGenVertexArrays(1, &m_Vao);
    glBindVertexArray(m_Vao);
}

void InfinityGridMaterial::Apply(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                                 std::shared_ptr<Light> light) {
    m_Shader->Bind();
    glBindVertexArray(m_Vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    m_Shader->SetMat4("u_Model", model);
    m_Shader->SetMat4("u_View", view);
    m_Shader->SetMat4("u_Projection", projection);

    glm::mat4 invView = glm::inverse(view);
    glm::vec3 camPos = glm::vec3(invView[3]);
    m_Shader->SetVec3("u_CameraPosition", camPos);
}

std::shared_ptr<Shader> InfinityGridMaterial::GetShader() const {
    return m_Shader;
}
