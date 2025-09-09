#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <optional>
#include <pure/PureMesh.hpp>
#include <pure/PurePicker.hpp>
#include <pure/PureTypes.hpp>
#include <unordered_set>

namespace pure {

static inline glm::vec3 closestPointOnSegment(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, float& t01) {
    glm::vec3 ab = b - a;
    float ab2 = glm::dot(ab, ab);
    if (ab2 <= 1e-12f) {
        t01 = 0.f;
        return a;
    }
    float t = glm::dot(p - a, ab) / ab2;
    t01 = std::clamp(t, 0.f, 1.f);
    return a + ab * t01;
}

// Möller–Trumbore
bool PurePicker::rayTriangle(const glm::vec3& ro, const glm::vec3& rd, const glm::vec3& a, const glm::vec3& b,
                             const glm::vec3& c, float& t, float& u, float& v) {
    const float EPS = 1e-7f;
    glm::vec3 e1 = b - a;
    glm::vec3 e2 = c - a;
    glm::vec3 p = glm::cross(rd, e2);
    float det = glm::dot(e1, p);
    if (fabsf(det) < EPS) return false;
    float invDet = 1.0f / det;
    glm::vec3 s = ro - a;
    u = glm::dot(s, p) * invDet;
    if (u < 0.0f || u > 1.0f) return false;
    glm::vec3 q = glm::cross(s, e1);
    v = glm::dot(rd, q) * invDet;
    if (v < 0.0f || u + v > 1.0f) return false;
    t = glm::dot(e2, q) * invDet;
    return (t > EPS);
}

void PurePicker::screenRay(float mx, float my, glm::vec3& ro, glm::vec3& rd) const {
    // NDC
    float x = (2.f * mx) / float(m_vpW) - 1.f;
    float y = 1.f - (2.f * my) / float(m_vpH);
    glm::vec4 pNear = m_invViewProj * glm::vec4(x, y, -1.f, 1.f);
    glm::vec4 pFar = m_invViewProj * glm::vec4(x, y, 1.f, 1.f);
    pNear /= pNear.w;
    pFar /= pFar.w;
    ro = glm::vec3(pNear);
    rd = glm::normalize(glm::vec3(pFar - pNear));
}

float PurePicker::pixelToWorld(float px, float depthWorld) const {
    // simple heuristic: length at depth = projScale * px
    // compute ray through center and +px in x, intersect plane orthogonal to view at depthWorld
    // here we approximate with: world_per_px ≈ depth * tan(fov_x/2) * 2 / vpW
    // better: query your projection params; for now a stable fallback:
    float worldPerPx = std::max(0.001f, depthWorld * 2.0f / float(m_vpW));
    return worldPerPx * px * m_dpi;
}

const PurePicker::EdgeCache& PurePicker::getEdgeCache(const PureMesh* m) {
    // simple linear search
    for (auto& c : m_edgeCaches)
        if (c.mesh == m) return c;
    // build
    EdgeCache cache;
    cache.mesh = m;
    const auto& idx = m->Indices();
    // undirected edge set
    struct Key {
        uint32_t a, b;
        bool operator==(const Key& o) const {
            return a == o.a && b == o.b;
        }
    };
    struct Hash {
        size_t operator()(const Key& k) const {
            return (size_t(k.a) << 32) ^ size_t(k.b);
        }
    };
    std::unordered_set<Key, Hash> set;
    auto addEdge = [&](uint32_t i, uint32_t j) {
        uint32_t a = std::min(i, j), b = std::max(i, j);
        Key k{a, b};
        if (set.insert(k).second) cache.edges.push_back({a, b});
    };
    for (size_t i = 0; i + 2 < idx.size(); i += 3) {
        addEdge(idx[i + 0], idx[i + 1]);
        addEdge(idx[i + 1], idx[i + 2]);
        addEdge(idx[i + 2], idx[i + 0]);
    }
    m_edgeCaches.push_back(std::move(cache));
    return m_edgeCaches.back();
}

bool PurePicker::hitFace(const glm::vec3& ro, const glm::vec3& rd, glm::vec3& outHitPos) {
    float bestT = std::numeric_limits<float>::max();
    bool hit = false;
    if (!m_scene) return false;

    for (const auto& part : m_scene->Parts()) {
        if (!part.mesh) continue;
        const auto& V = part.mesh->Vertices();
        const auto& I = part.mesh->Indices();
        glm::mat4 T = part.model;
        for (size_t i = 0; i + 2 < I.size(); i += 3) {
            glm::vec3 a = glm::vec3(T * glm::vec4(V[I[i + 0]].position, 1));
            glm::vec3 b = glm::vec3(T * glm::vec4(V[I[i + 1]].position, 1));
            glm::vec3 c = glm::vec3(T * glm::vec4(V[I[i + 2]].position, 1));
            float t, u, v;
            if (rayTriangle(ro, rd, a, b, c, t, u, v)) {
                if (t < bestT) {
                    bestT = t;
                    outHitPos = ro + rd * t;
                    hit = true;
                }
            }
        }
    }
    return hit;
}

bool PurePicker::snapVertex(const glm::vec3& ro, const glm::vec3& rd, glm::vec3& outHitPos) {
    if (!m_scene) return false;
    bool found = false;
    float bestDepth = std::numeric_limits<float>::max();
    float bestScreenDist = std::numeric_limits<float>::max();

    // project vertex onto ray (closest point) then measure perpendicular distance in world,
    // convert to pixels with heuristic at that depth; accept if within m_snapPx
    for (const auto& part : m_scene->Parts()) {
        if (!part.mesh) continue;
        const auto& V = part.mesh->Vertices();
        glm::mat4 T = part.model;

        for (const auto& v : V) {
            glm::vec3 p = glm::vec3(T * glm::vec4(v.position, 1));
            // distance point→ray
            glm::vec3 w = p - ro;
            float t = glm::dot(w, rd);
            glm::vec3 q = ro + rd * t;
            float dWorld = glm::length(p - q);
            float wpx = pixelToWorld(m_snapPx, std::max(0.001f, t));
            if (dWorld <= wpx + 1e-3f) {
                // prefer closer in depth, then closer in screen-distance
                float screenDist = dWorld / std::max(1e-6f, wpx);
                if (t < bestDepth || (fabsf(t - bestDepth) < 1e-4f && screenDist < bestScreenDist)) {
                    bestDepth = t;
                    bestScreenDist = screenDist;
                    outHitPos = p;
                    found = true;
                }
            }
        }
    }
    return found;
}

bool PurePicker::snapEdge(const glm::vec3& ro, const glm::vec3& rd, glm::vec3& outHitPos,
                          std::optional<Edge>& outEdge) {
    if (!m_scene) return false;
    bool found = false;
    float bestDepth = std::numeric_limits<float>::max();
    float bestScreenDist = std::numeric_limits<float>::max();

    for (const auto& part : m_scene->Parts()) {
        if (!part.mesh) continue;
        const auto& V = part.mesh->Vertices();
        glm::mat4 T = part.model;
        const auto& cache = getEdgeCache(part.mesh.get());

        for (const auto& e : cache.edges) {
            glm::vec3 a = glm::vec3(T * glm::vec4(V[e.i0].position, 1));
            glm::vec3 b = glm::vec3(T * glm::vec4(V[e.i1].position, 1));

            // Ray to segment: nearest point on ray and on segment -> use the point on segment for snap,
            // estimate "depth" by param along ray
            // compute closest point on ray: q = ro + rd * t
            // we approximate by projecting segment endpoints to the ray midpoint
            // better: use Ericson's segment-segment (ray as long segment)
            // Simple: project segment midpoint to ray
            glm::vec3 mid = 0.5f * (a + b);
            float t;
            glm::vec3 q = ro + rd * std::max(0.f, glm::dot(mid - ro, rd));
            // screen radius in world at depth t
            float wpx = pixelToWorld(m_snapPx, glm::length(q - ro));
            // distance from ray to segment: compute closest point on segment to q
            float dummy;
            glm::vec3 spt = closestPointOnSegment(q, a, b, dummy);
            float dWorld = glm::length(spt - q);
            if (dWorld <= wpx + 1e-3f) {
                float screenDist = dWorld / std::max(1e-6f, wpx);
                float depth = glm::length(q - ro);
                if (depth < bestDepth || (fabsf(depth - bestDepth) < 1e-4f && screenDist < bestScreenDist)) {
                    bestDepth = depth;
                    bestScreenDist = screenDist;
                    outHitPos = spt;
                    outEdge = pure::Edge{a, b};
                    found = true;
                }
            }
        }
    }
    return found;
}

bool PurePicker::Pick(float mouseX, float mouseY, SnapType snap, glm::vec3& outHitPos, std::optional<Edge>& outEdge) {
    outEdge.reset();
    glm::vec3 ro, rd;
    screenRay(mouseX, mouseY, ro, rd);
    snapVertex(ro, rd, outHitPos);

    return true;

    // switch (snap) {
    //     case SnapType::Vertex:
    //         if (snapVertex(ro, rd, outHitPos)) return true;
    //         // fallback to face
    //         return hitFace(ro, rd, outHitPos);
    //
    //     case SnapType::Edge:
    //         if (snapEdge(ro, rd, outHitPos, outEdge)) return true;
    //         // fallback: face hit
    //         return hitFace(ro, rd, outHitPos);
    //
    //     case SnapType::Face:
    //     default:
    //         return hitFace(ro, rd, outHitPos);
    // }
}

}  // namespace pure
