#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <pure/PureAxis.hpp>
#include <vector>

namespace pure {

PureAxis::~PureAxis() {
    Shutdown();
}

bool PureAxis::Initialize(float axisLength, float headLength, float headWidth) {
    m_Shader.BuildUnlit();
    BuildBuffers(axisLength, headLength, headWidth);
    return true;
}

void PureAxis::Shutdown() {
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

void PureAxis::BuildBuffers(float axisLength, float headLength, float headWidth) {
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

    // X-axis (R)
    {
        glm::vec3 a(0, 0, 0), b(axisLength, 0, 0);
        PushLine(a, b, colX);
        // Pfeilspitze als zwei schräge Linien
        PushLine(b, b + glm::vec3(-headLength, +headWidth, 0), colX);
        PushLine(b, b + glm::vec3(-headLength, -headWidth, 0), colX);
    }
    // Y-axis (G)
    {
        glm::vec3 a(0, 0, 0), b(0, axisLength, 0);
        PushLine(a, b, colY);
        PushLine(b, b + glm::vec3(+headWidth, -headLength, 0), colY);
        PushLine(b, b + glm::vec3(-headWidth, -headLength, 0), colY);
    }
    // Z-axis (B)
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
    if (!m_Vao || m_VertexCount <= 0) return;

    m_Shader.Bind();

    glm::mat4 mvp = proj * view * m_Model;
    m_Shader.SetMat4("uMVP", mvp);

    glBindVertexArray(m_Vao);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, m_VertexCount);
    glBindVertexArray(0);
}

}  // namespace pure
