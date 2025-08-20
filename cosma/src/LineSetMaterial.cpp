#include <material/LineSetMaterial.hpp>
#include <material/Shader.hpp>

#include "assets/lineset_frag.h"
#include "assets/lineset_vert.h"

LineSetMaterial::LineSetMaterial() {
    m_Shader = std::make_shared<Shader>(lineset_vert_glsl, lineset_frag_glsl);
}

void LineSetMaterial::Apply(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                            std::shared_ptr<Light> /*light*/) {
    m_Shader->Bind();
    m_Shader->SetMat4("u_Model", model);
    m_Shader->SetMat4("u_View", view);
    m_Shader->SetMat4("u_Projection", projection);
}

std::shared_ptr<Shader> LineSetMaterial::GetShader() const {
    return m_Shader;
}
