#pragma once
#include <glad.h>

#include <glm/glm.hpp>
#include <string>

class Shader {
   public:
    unsigned int ID;

    Shader(const std::string& vert, const std::string& frag);
    virtual ~Shader() = default;

    // Activate shader
    void Bind();

    // Set data to shader
    void SetMat4(const std::string& name, const glm::mat4& matrix);
    void SetVec3(const std::string& name, const glm::vec3& value);
    void SetFloat(const std::string& name, float value);
    void SetBool(const std::string& name, bool value);

    unsigned int GetInt(const std::string& name) const;

   private:
    std::string readShaderFile(const std::string& path);
    void compileShader(const std::string& vertexCode, const std::string& fragmentCode);
};
