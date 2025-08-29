#pragma once
#include <glad.h>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <string>

namespace pure {

class PureShader {
   public:
    PureShader() = default;
    ~PureShader();

    bool BuildPhong(std::string* errorMessage = nullptr);

    void Use() const {
        glUseProgram(m_Program);
    }
    GLuint Id() const {
        return m_Program;
    }

    // Uniforms
    void SetMat4(const char* name, const glm::mat4& m) const;
    void SetMat3(const char* name, const glm::mat3& m) const;
    void SetVec3(const char* name, const glm::vec3& v) const;

   private:
    GLuint Compile(GLenum type, const char* src, std::string* errorMessage);
    bool Link(GLuint vs, GLuint fs, std::string* errorMessage);

    GLuint m_Program = 0;
};

}  // namespace pure
