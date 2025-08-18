// clang-format off
#include <glad.h>
// clang-format on
#include <geometry/Mesh.hpp>
#include <glm/gtc/constants.hpp>

#include "geometry/BaseGeometry.hpp"

Mesh::Mesh() : m_Vao(0), m_Vbo(0), m_Ebo(0), BaseGeometry() {
}

Mesh::~Mesh() {
    deleteBuffers();
}

void Mesh::AddIndex(unsigned int idx) {
    m_Indices.push_back(idx);
    m_Dirty = true;
}

void Mesh::AddTriangle(unsigned int v1, unsigned int v2, unsigned int v3) {
    m_Indices.push_back(v1);
    m_Indices.push_back(v2);
    m_Indices.push_back(v3);
    m_Dirty = true;
}

void Mesh::Upload() {
    // ignore if the data has not changed
    if (!m_Dirty) return;

    // Clean up old objects if necessary
    deleteBuffers();

    glGenVertexArrays(1, &m_Vao);
    glBindVertexArray(m_Vao);

    glGenBuffers(1, &m_Vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), m_Vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_Ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), m_Indices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)Vertex::PositionOffset());
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)Vertex::NormalOffset());
    glEnableVertexAttribArray(1);

    // Color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)Vertex::ColorOffset());
    glEnableVertexAttribArray(2);

    // TexCoord
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)Vertex::TexCoordOffset());
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
    m_Dirty = false;
}

const std::vector<unsigned int>& Mesh::GetIndices() const {
    return m_Indices;
}

void Mesh::Render() const {
    glBindVertexArray(m_Vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_INT, 0);
}

void Mesh::deleteBuffers() {
    if (m_Vao) glDeleteVertexArrays(1, &m_Vao);
    if (m_Vbo) glDeleteBuffers(1, &m_Vbo);
    if (m_Ebo) glDeleteBuffers(1, &m_Ebo);
}

void Mesh::RecalculateNormals() {
    for (auto& v : m_Vertices) {
        v.SetNormal(glm::vec3(0.0f));
    }

    for (size_t i = 0; i + 2 < m_Indices.size(); i += 3) {
        unsigned int i0 = m_Indices[i];
        unsigned int i1 = m_Indices[i + 1];
        unsigned int i2 = m_Indices[i + 2];

        const glm::vec3& p0 = m_Vertices[i0].GetPosition();
        const glm::vec3& p1 = m_Vertices[i1].GetPosition();
        const glm::vec3& p2 = m_Vertices[i2].GetPosition();

        glm::vec3 edge1 = p1 - p0;
        glm::vec3 edge2 = p2 - p0;

        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        m_Vertices[i0].SetNormal(normal);
        m_Vertices[i1].SetNormal(normal);
        m_Vertices[i2].SetNormal(normal);
    }
    m_Dirty = true;
}
