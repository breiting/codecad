#pragma once
#include <glad.h>

#include <glm/glm.hpp>
#include <vector>

namespace pure {

struct PureVertex {
    glm::vec3 position;
    glm::vec3 normal;
};

struct PureAabb {
    glm::vec3 min{0}, max{0};
    bool valid{false};
    void Reset() {
        valid = false;
    }
    void Expand(const glm::vec3& p) {
        if (!valid) {
            min = max = p;
            valid = true;
            return;
        }
        min = glm::min(min, p);
        max = glm::max(max, p);
    }
};

class PureMesh {
   public:
    PureMesh() = default;
    ~PureMesh();

    void Upload(const std::vector<PureVertex>& vertices, const std::vector<unsigned>& indices);

    void Draw() const;

    bool Empty() const {
        return m_IndexCount == 0;
    }
    PureAabb Bounds() const {
        return m_Bounds;
    }

   private:
    GLuint m_Vao = 0, m_Vbo = 0, m_Ebo = 0;
    GLsizei m_IndexCount = 0;
    PureAabb m_Bounds;
};

}  // namespace pure
