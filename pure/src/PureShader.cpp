#include <glm/gtc/type_ptr.hpp>
#include <pure/PureShader.hpp>
#include <vector>

namespace pure {

static const char* kPhongVS = R"(#version 330 core
layout(location=0) in vec3 in_position;
layout(location=1) in vec3 in_normal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform mat3 u_normalMatrix;

out vec3 v_normal;
out vec3 v_worldPos;

void main() {
  vec4 wp = u_model * vec4(in_position, 1.0);
  v_worldPos = wp.xyz;
  v_normal = normalize(u_normalMatrix * in_normal);
  gl_Position = u_proj * u_view * wp;
}
)";

static const char* kPhongFS = R"(#version 330 core
in vec3 v_normal;
in vec3 v_worldPos;

out vec4 fragColor;

uniform vec3 u_baseColor; // 0..1
uniform vec3 u_camPos;
uniform vec3 u_lightDir;  // Headlight: from camera view direction

void main() {
  vec3 N = normalize(v_normal);
  vec3 L = normalize(-u_lightDir);

  // Ambient
  vec3 ambient = 0.2 * u_baseColor;

  // Diffuse
  float ndotl = max(dot(N, L), 0.0);
  vec3 diffuse = ndotl * u_baseColor;

  // Specular (Blinn-Phong)
  vec3 V = normalize(u_camPos - v_worldPos);
  vec3 H = normalize(L + V);
  float nh = max(dot(N, H), 0.0);
  float spec = pow(nh, 32.0);
  vec3 specular = 0.2 * spec * vec3(1.0);

  fragColor = vec4(ambient + diffuse + specular, 1.0);
}
)";

PureShader::~PureShader() {
    if (m_Program) glDeleteProgram(m_Program);
}

GLuint PureShader::Compile(GLenum type, const char* src, std::string* errorMessage) {
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> buf(len);
        glGetShaderInfoLog(sh, len, nullptr, buf.data());
        if (errorMessage) *errorMessage = std::string(buf.begin(), buf.end());
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}

bool PureShader::Link(GLuint vs, GLuint fs, std::string* errorMessage) {
    m_Program = glCreateProgram();
    glAttachShader(m_Program, vs);
    glAttachShader(m_Program, fs);
    glLinkProgram(m_Program);
    GLint ok = 0;
    glGetProgramiv(m_Program, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> buf(len);
        glGetProgramInfoLog(m_Program, len, nullptr, buf.data());
        if (errorMessage) *errorMessage = std::string(buf.begin(), buf.end());
        glDeleteProgram(m_Program);
        m_Program = 0;
        return false;
    }
    glDetachShader(m_Program, vs);
    glDeleteShader(vs);
    glDetachShader(m_Program, fs);
    glDeleteShader(fs);
    return true;
}

bool PureShader::BuildPhong(std::string* errorMessage) {
    GLuint vs = Compile(GL_VERTEX_SHADER, kPhongVS, errorMessage);
    if (!vs) return false;
    GLuint fs = Compile(GL_FRAGMENT_SHADER, kPhongFS, errorMessage);
    if (!fs) {
        glDeleteShader(vs);
        return false;
    }
    return Link(vs, fs, errorMessage);
}

void PureShader::SetMat4(const char* name, const glm::mat4& m) const {
    GLint loc = glGetUniformLocation(m_Program, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}
void PureShader::SetMat3(const char* name, const glm::mat3& m) const {
    GLint loc = glGetUniformLocation(m_Program, name);
    glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}
void PureShader::SetVec3(const char* name, const glm::vec3& v) const {
    GLint loc = glGetUniformLocation(m_Program, name);
    glUniform3fv(loc, 1, glm::value_ptr(v));
}

}  // namespace pure
