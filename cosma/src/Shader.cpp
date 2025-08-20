#include <fstream>
#include <iostream>
#include <material/Shader.hpp>
#include <sstream>

Shader::Shader(const std::string& vert, const std::string& frag) {
    compileShader(std::string(vert), std::string(frag));
}

std::string Shader::readShaderFile(const std::string& path) {
    std::ifstream file(path);
    std::stringstream fileStream;

    if (!file.is_open()) {
        std::cerr << "Fehler beim Öffnen der Datei: " << path << std::endl;
        return "";
    }

    fileStream << file.rdbuf();
    return fileStream.str();
}

void Shader::compileShader(const std::string& vertexCode, const std::string& fragmentCode) {
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cerr << "Error compiling vertex shader: " << infoLog << std::endl;
    }

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        std::cerr << "Error compiling fragment shader: " << infoLog << std::endl;
    }

    // Shader-Programm creation
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        std::cerr << "Error binding shader: " << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::Bind() {
    glUseProgram(ID);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& matrix) {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::SetFloat(const std::string& name, float value) {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetBool(const std::string& name, bool value) {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

unsigned int Shader::GetInt(const std::string& name) const {
    return glGetUniformLocation(ID, name.c_str());
}
