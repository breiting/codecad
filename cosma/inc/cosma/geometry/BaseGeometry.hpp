#pragma once
#include <geometry/Vertex.hpp>
#include <glm/glm.hpp>
#include <vector>

class BaseGeometry {
   public:
    virtual void Upload() = 0;
    virtual void Render() const = 0;

    BaseGeometry();
    virtual ~BaseGeometry() = default;

    void AddVertex(const Vertex& v);
    size_t VertexCount() const;
    const std::vector<Vertex>& GetVertices() const;

   protected:
    std::vector<Vertex> m_Vertices;
    bool m_Dirty;
};
