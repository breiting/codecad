#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <pure/PureAxis.hpp>
#include <vector>

namespace pure {

static unsigned int Compile(unsigned int type, const char* src, std::string* err) {
    unsigned int sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    int ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        if (err) {
            char log[2048];
            glGetShaderInfoLog(sh, 2048, nullptr, log);
            *err = log;
        }
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}
static unsigned int Link(unsigned int vs, unsigned int fs, std::string* err) {
    unsigned int p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    int ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        if (err) {
            char log[2048];
            glGetProgramInfoLog(p, 2048, nullptr, log);
            *err = log;
        }
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

PureAxis::~PureAxis() {
    Shutdown();
}

bool PureAxis::Initialize(float axisLength, float headLength, float headWidth, std::string* errorMessage) {
    if (!BuildShader(errorMessage)) return false;
    BuildBuffers(axisLength, headLength, headWidth);
    return true;
}

void PureAxis::Shutdown() {
    if (m_Prog) {
        glDeleteProgram(m_Prog);
        m_Prog = 0;
    }
    if (m_Vbo) {
        glDeleteBuffers(1, &m_Vbo);
        m_Vbo = 0;
    }
    if (m_Vao) {
        glDeleteVertexArrays(1, &m_Vao);
        m_Vao = 0;
    }
}

void PureAxis::SetModel(const glm::mat4& model) {
    m_Model = model;
}

bool PureAxis::BuildShader(std::string* errorMessage) {
    // Unlit-Farbshader, zeichnet Linien
    const char* vsSrc = R"(#version 330 core
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inCol;
uniform mat4 uMVP;
out vec3 vCol;
void main() {
  vCol = inCol;
  gl_Position = uMVP * vec4(inPos, 1.0);
})";
    const char* fsSrc = R"(#version 330 core
in vec3 vCol;
out vec4 fragColor;
void main() {
  fragColor = vec4(vCol, 1.0);
})";
    std::string err;
    unsigned int vs = Compile(GL_VERTEX_SHADER, vsSrc, &err);
    if (!vs) {
        if (errorMessage) *errorMessage = "Axis VS: " + err;
        return false;
    }
    unsigned int fs = Compile(GL_FRAGMENT_SHADER, fsSrc, &err);
    if (!fs) {
        glDeleteShader(vs);
        if (errorMessage) *errorMessage = "Axis FS: " + err;
        return false;
    }
    m_Prog = Link(vs, fs, &err);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!m_Prog) {
        if (errorMessage) *errorMessage = "Axis Link: " + err;
        return false;
    }
    return true;
}

void PureAxis::BuildBuffers(float axisLength, float headLength, float headWidth) {
    // Wir bauen Linien mit Pfeilspitzen: pro Achse 1 Liniensegment + 2 Kopfsegmente.
    // Format: pos.xyz, col.rgb
    struct V {
        float x, y, z, r, g, b;
    };
    std::vector<V> v;

    auto PushLine = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c) {
        v.push_back({a.x, a.y, a.z, c.r, c.g, c.b});
        v.push_back({b.x, b.y, b.z, c.r, c.g, c.b});
    };

    const glm::vec3 colX(1.0f, 0.2f, 0.2f);
    const glm::vec3 colY(0.2f, 0.9f, 0.2f);
    const glm::vec3 colZ(0.2f, 0.5f, 1.0f);

    // X-Achse (R)
    {
        glm::vec3 a(0, 0, 0), b(axisLength, 0, 0);
        PushLine(a, b, colX);
        // Pfeilspitze als zwei schräge Linien
        PushLine(b, b + glm::vec3(-headLength, +headWidth, 0), colX);
        PushLine(b, b + glm::vec3(-headLength, -headWidth, 0), colX);
    }
    // Y-Achse (G)
    {
        glm::vec3 a(0, 0, 0), b(0, axisLength, 0);
        PushLine(a, b, colY);
        PushLine(b, b + glm::vec3(+headWidth, -headLength, 0), colY);
        PushLine(b, b + glm::vec3(-headWidth, -headLength, 0), colY);
    }
    // Z-Achse (B)
    {
        glm::vec3 a(0, 0, 0), b(0, 0, axisLength);
        PushLine(a, b, colZ);
        PushLine(b, b + glm::vec3(+headWidth, 0, -headLength), colZ);
        PushLine(b, b + glm::vec3(-headWidth, 0, -headLength), colZ);
    }

    m_VertexCount = static_cast<int>(v.size());

    glGenVertexArrays(1, &m_Vao);
    glGenBuffers(1, &m_Vbo);
    glBindVertexArray(m_Vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(V), v.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
}

void PureAxis::Render(const glm::mat4& view, const glm::mat4& proj) const {
    if (!m_Prog || !m_Vao || m_VertexCount <= 0) return;

    // Depth-Test darf an bleiben – Achsen „leben“ in der Szene.
    glUseProgram(m_Prog);

    glm::mat4 mvp = proj * view * m_Model;
    GLint loc = glGetUniformLocation(m_Prog, "uMVP");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mvp));

    glBindVertexArray(m_Vao);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, m_VertexCount);
    glBindVertexArray(0);

    glUseProgram(0);
}

}  // namespace pure
