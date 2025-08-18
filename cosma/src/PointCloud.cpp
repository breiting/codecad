// clang-format off
#include <glad.h>
// clang-format on
#include <geometry/PointCloud.hpp>
#include <glm/gtc/constants.hpp>

PointCloud::PointCloud() : m_Vao(0), m_Vbo(0), BaseGeometry() {
}

PointCloud::~PointCloud() {
    deleteBuffers();
}

void PointCloud::Upload() {
    // ignore if the data has not changed
    if (!m_Dirty) return;

    // Clean up old objects if necessary
    deleteBuffers();

    glGenVertexArrays(1, &m_Vao);
    glBindVertexArray(m_Vao);

    glGenBuffers(1, &m_Vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), m_Vertices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)Vertex::PositionOffset());
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)Vertex::NormalOffset());
    glEnableVertexAttribArray(1);

    // Color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)Vertex::ColorOffset());
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    m_Dirty = false;
}

void PointCloud::Render() const {
    glBindVertexArray(m_Vao);
    glDrawArrays(GL_POINTS, 0, m_Vertices.size());
}

void PointCloud::deleteBuffers() {
    if (m_Vao) glDeleteVertexArrays(1, &m_Vao);
    if (m_Vbo) glDeleteBuffers(1, &m_Vbo);
}
