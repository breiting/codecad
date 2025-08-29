#include <pure/PureMesh.hpp>

namespace pure {

PureMesh::~PureMesh() {
    if (m_Ebo) glDeleteBuffers(1, &m_Ebo);
    if (m_Vbo) glDeleteBuffers(1, &m_Vbo);
    if (m_Vao) glDeleteVertexArrays(1, &m_Vao);
}

void PureMesh::Upload(const std::vector<PureVertex>& vertices, const std::vector<unsigned>& indices) {
    if (!m_Vao) glGenVertexArrays(1, &m_Vao);
    if (!m_Vbo) glGenBuffers(1, &m_Vbo);
    if (!m_Ebo) glGenBuffers(1, &m_Ebo);

    glBindVertexArray(m_Vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(PureVertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PureVertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PureVertex), (void*)sizeof(glm::vec3));

    glBindVertexArray(0);
    m_IndexCount = static_cast<GLsizei>(indices.size());

    // Bounds
    m_Bounds.Reset();
    for (auto& v : vertices) m_Bounds.Expand(v.position);
}

void PureMesh::Draw() const {
    if (!m_IndexCount) return;
    glBindVertexArray(m_Vao);
    glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

}  // namespace pure
