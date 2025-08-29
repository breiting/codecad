#pragma once
// clang-format off
#include <glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <glm/mat4x4.hpp>
#include <string>

namespace pure {

class PureAxis {
   public:
    PureAxis() = default;
    ~PureAxis();

    bool Initialize(float axisLength = 100.0f, float headLength = 12.0f, float headWidth = 6.0f,
                    std::string* errorMessage = nullptr);

    void Shutdown();

    // Model-Matrix
    void SetModel(const glm::mat4& model);

    void Render(const glm::mat4& view, const glm::mat4& proj) const;

   private:
    bool BuildShader(std::string* errorMessage);
    void BuildBuffers(float axisLength, float headLength, float headWidth);

   private:
    unsigned int m_Vao = 0;
    unsigned int m_Vbo = 0;
    unsigned int m_Prog = 0;
    int m_VertexCount = 0;

    glm::mat4 m_Model{1.0f};
};

}  // namespace pure
