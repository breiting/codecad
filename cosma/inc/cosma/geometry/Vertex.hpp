#pragma once

#include <glm/glm.hpp>

class Vertex {
   public:
    Vertex() = default;
    Vertex(const glm::vec3 &pos, const glm::vec3 &normal = glm::vec3(0.0f), const glm::vec3 &col = glm::vec3(0.0f),
           const glm::vec2 &texCoord = glm::vec2(0.0f))
        : m_Position(pos), m_Normal(normal), m_Color(col), m_TexCoord(texCoord) {
    }

    glm::vec3 GetPosition() const {
        return m_Position;
    }
    glm::vec3 GetNormal() const {
        return m_Normal;
    }
    glm::vec3 GetColor() const {
        return m_Color;
    }
    glm::vec2 GetTexCoord() const {
        return m_TexCoord;
    }

    void SetPosition(const glm::vec3 &pos) {
        m_Position = pos;
    }
    void SetNormal(const glm::vec3 &normal) {
        m_Normal = normal;
    }
    void SetColor(const glm::vec3 &color) {
        m_Color = color;
    }
    void SetTexCoord(const glm::vec2 &texCoord) {
        m_TexCoord = texCoord;
    }

    // Required for OpenGL rendering
    static constexpr size_t PositionOffset() {
        return offsetof(Vertex, m_Position);
    }
    static constexpr size_t NormalOffset() {
        return offsetof(Vertex, m_Normal);
    }
    static constexpr size_t ColorOffset() {
        return offsetof(Vertex, m_Color);
    }
    static constexpr size_t TexCoordOffset() {
        return offsetof(Vertex, m_TexCoord);
    }

   private:
    glm::vec3 m_Position;
    glm::vec3 m_Normal;
    glm::vec3 m_Color;
    glm::vec2 m_TexCoord;
};
