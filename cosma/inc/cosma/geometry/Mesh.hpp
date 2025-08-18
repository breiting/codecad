#pragma once

#include <core/Texture.hpp>
#include <geometry/BaseGeometry.hpp>
#include <glm/glm.hpp>
#include <vector>

class Mesh : public BaseGeometry {
   public:
    Mesh();
    ~Mesh();

    void AddIndex(unsigned int idx);
    void AddTriangle(unsigned int v1, unsigned int v2, unsigned int v3);

    void Upload() override;
    void Render() const override;

    const std::vector<unsigned int>& GetIndices() const;
    void RecalculateNormals();

   private:
    void deleteBuffers();

   private:
    unsigned int m_Vao, m_Vbo, m_Ebo;
    std::vector<unsigned int> m_Indices;
};
