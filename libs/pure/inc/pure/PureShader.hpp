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

    unsigned Id() const {
        return m_Id;
    }

    // Activate/use shader
    void Bind() const;

    // Load and build a pre-defined shader
    void BuildPhong();
    void BuildUnlit();

    // Set data to shader
    void SetMat3(const std::string& name, const glm::mat3& m) const;
    void SetMat4(const std::string& name, const glm::mat4& matrix) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetBool(const std::string& name, bool value) const;

    unsigned int GetInt(const std::string& name) const;

   private:
    void CompileShader(const std::string& vertexCode, const std::string& fragmentCode);

   private:
    GLuint m_Id;
};

}  // namespace pure
