#include <pure/PureMesh.hpp>

using namespace std;

namespace pure {

PureMesh::~PureMesh() {
    if (m_Ebo) glDeleteBuffers(1, &m_Ebo);
    if (m_Vbo) glDeleteBuffers(1, &m_Vbo);
    if (m_Vao) glDeleteVertexArrays(1, &m_Vao);
}

void PureMesh::Upload(std::vector<PureVertex>& vertices, const std::vector<unsigned>& indices,
                      bool recalculateNormals) {
    if (recalculateNormals) {
        for (auto& v : vertices) {
            v.normal = glm::vec3(0.0f);
        }

        for (size_t i = 0; i + 2 < indices.size(); i += 3) {
            unsigned int i0 = indices[i];
            unsigned int i1 = indices[i + 1];
            unsigned int i2 = indices[i + 2];

            const glm::vec3& p0 = vertices[i0].position;
            const glm::vec3& p1 = vertices[i1].position;
            const glm::vec3& p2 = vertices[i2].position;

            glm::vec3 edge1 = p1 - p0;
            glm::vec3 edge2 = p2 - p0;

            glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

            vertices[i0].normal = normal;
            vertices[i1].normal = normal;
            vertices[i2].normal = normal;
        }
    }

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
