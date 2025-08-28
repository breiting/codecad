#pragma once
#include <geometry/Vertex.hpp>
#include <glm/glm.hpp>
#include <vector>

#include "core/Bounds.hpp"

class BaseGeometry {
   public:
    virtual void Upload() = 0;
    virtual void Render() const = 0;

    BaseGeometry();
    virtual ~BaseGeometry() = default;

    void AddVertex(const Vertex& v);
    size_t VertexCount() const;
    const std::vector<Vertex>& GetVertices() const;

    AABB ComputeLocalAABB() const;

   protected:
    std::vector<Vertex> m_Vertices;
    bool m_Dirty;

    mutable AABB m_CachedAABB;
};
