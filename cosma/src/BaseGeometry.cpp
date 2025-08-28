// clang-format off
#include <glad.h>
// clang-format on
#include <geometry/BaseGeometry.hpp>

BaseGeometry::BaseGeometry() : m_Dirty(true) {
}

void BaseGeometry::AddVertex(const Vertex& v) {
    m_Vertices.push_back(v);
    m_Dirty = true;
}

size_t BaseGeometry::VertexCount() const {
    return m_Vertices.size();
}

const std::vector<Vertex>& BaseGeometry::GetVertices() const {
    return m_Vertices;
}

AABB BaseGeometry::ComputeLocalAABB() const {
    if (m_CachedAABB.valid) return m_CachedAABB;

    AABB box;
    for (const auto& v : m_Vertices) {
        box.Expand(v.GetPosition());
    }
    m_CachedAABB = box;  // cache
    return box;
}
