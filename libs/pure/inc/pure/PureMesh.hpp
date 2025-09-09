#pragma once
#include <glad.h>

#include <glm/glm.hpp>
#include <pure/PureTypes.hpp>
#include <vector>

namespace pure {

class PureMesh {
   public:
    PureMesh() = default;
    ~PureMesh();

    void Upload(std::vector<PureVertex>& vertices, const std::vector<unsigned>& indices,
                bool recalculateNormals = true);

    void Draw() const;

    bool Empty() const {
        return m_Indices.empty();
    }
    PureAabb Bounds() const {
        return m_Bounds;
    }
    const std::vector<PureVertex> Vertices() const {
        return m_Vertices;
    }

    const std::vector<unsigned> Indices() const {
        return m_Indices;
    }

   private:
    std::vector<PureVertex> m_Vertices;
    std::vector<unsigned> m_Indices;
    GLuint m_Vao = 0, m_Vbo = 0, m_Ebo = 0;
    PureAabb m_Bounds;
};

}  // namespace pure
