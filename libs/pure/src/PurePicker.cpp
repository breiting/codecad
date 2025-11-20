#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <optional>
#include <pure/PureMesh.hpp>
#include <pure/PurePicker.hpp>
#include <pure/PureTypes.hpp>
#include <unordered_set>

#include "imgui.h"

namespace pure {

static inline glm::vec3 closestPointOnSegment(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, float& t01) {
    glm::vec3 ab = b - a;
    float ab2 = glm::dot(ab, ab);
    if (ab2 <= 1e-12f) {
        t01 = 0.f;
        return a;
    }
    float t = glm::dot(p - a, ab) / ab2;
    t01 = glm::clamp(t, 0.f, 1.f);
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
    float x = (2.f * mx) / float(m_ViewportWidth) - 1.f;
    float y = 1.f - (2.f * my) / float(m_ViewportHeight);
    glm::vec4 pNear = m_InvViewProj * glm::vec4(x, y, -1.f, 1.f);
    glm::vec4 pFar = m_InvViewProj * glm::vec4(x, y, 1.f, 1.f);
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
    float worldPerPx = std::max(0.001f, depthWorld * 2.0f / float(m_ViewportWidth));
    return worldPerPx * px * m_Dpi;
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

bool PurePicker::snapVertex(const glm::vec3& ro, const glm::vec3& rd, glm::vec3& outHitPos) {
    if (!m_Scene) return false;
    bool found = false;
    float bestDepth = std::numeric_limits<float>::max();
    float bestScreenDist = std::numeric_limits<float>::max();

    // project vertex onto ray (closest point) then measure perpendicular distance in world,
    // convert to pixels with heuristic at that depth; accept if within m_snapPx
    for (const auto& part : m_Scene->Parts()) {
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
            float wpx = pixelToWorld(m_SnapPx, std::max(0.001f, t));
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
    if (!m_Scene) return false;
    bool found = false;
    float bestDepth = std::numeric_limits<float>::max();
    float bestScreenDist = std::numeric_limits<float>::max();

    for (const auto& part : m_Scene->Parts()) {
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
            glm::vec3 q = ro + rd * std::max(0.f, glm::dot(mid - ro, rd));
            // screen radius in world at depth t
            float wpx = pixelToWorld(m_SnapPx, glm::length(q - ro));
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

PurePicker::HoverState PurePicker::GetHoverState() const {
    return m_Hover;
}

void PurePicker::UpdateHover(float mouseX, float mouseY) {
    glm::vec3 hitPos;
    std::optional<Edge> edge;

    // Try vertex snap first (sharpest), then edge
    glm::vec3 ro, rd;
    screenRay(mouseX, mouseY, ro, rd);

    m_Hover = {};

    if (snapVertex(ro, rd, hitPos)) {
        if (isVisibleWorldPoint(hitPos)) {
            m_Hover.kind = HoverKind::Vertex;
            m_Hover.pos = hitPos;
            return;
        }
    }

    if (snapEdge(ro, rd, hitPos, edge)) {
        bool vis = true;
        vis = isVisibleWorldPoint(hitPos);
        if (vis && edge) {
            glm::vec3 mid = 0.5f * (edge->a + edge->b);
            vis = isVisibleWorldPoint(mid);
        }
        if (vis) {
            m_Hover.kind = HoverKind::Edge;
            m_Hover.pos = hitPos;
            m_Hover.edge = edge;
            return;
        }
    }
    m_Hover = {};
}

bool PurePicker::sampleDepth(int sx, int sy, float& outDepth) const {
    // Guard
    if (sx < 0 || sy < 0 || sx >= m_ViewportWidth || sy >= m_ViewportHeight) return false;

    int iy = sy;
    iy = (m_ViewportHeight - 1) - sy;

    float depth = 1.0f;
    glReadPixels(sx, iy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    outDepth = depth;
    return true;
}

bool PurePicker::isVisibleWorldPoint(const glm::vec3& w) const {
    int sx, sy;
    float ndcDepth;
    if (!worldToScreen(w, sx, sy, ndcDepth)) return false;

    float zBuf;
    if (!sampleDepth(sx, sy, zBuf)) return true;  // Fallback: visible
    const float eps = 1.5f / 65535.0f;
    return (ndcDepth <= zBuf + eps);
}

bool PurePicker::worldToScreen(const glm::vec3& w, int& outX, int& outY, float& outDepth01) const {
    glm::vec4 clip = m_ViewProj * glm::vec4(w, 1.0f);
    if (clip.w <= 1e-6f) return false;
    glm::vec3 ndc = glm::vec3(clip) / clip.w;  // -1..1
    if (ndc.x < -1.f || ndc.x > 1.f || ndc.y < -1.f || ndc.y > 1.f || ndc.z < -1.f || ndc.z > 1.f) return false;

    outX = int((ndc.x * 0.5f + 0.5f) * m_ViewportWidth);
    outY = int((1.0f - (ndc.y * 0.5f + 0.5f)) * m_ViewportHeight);  // origin top-left
    outDepth01 = (ndc.z * 0.5f + 0.5f);                             // [-1,1] -> [0,1]
    return true;
}

void PurePicker::DrawHoverOverlay(ImDrawList* dl) const {
    if (!dl) return;

    const float px = 1.0f * m_Dpi;
    const ImU32 colPt = IM_COL32(255, 210, 64, 255);     // amber
    const ImU32 colPt2 = IM_COL32(0, 0, 0, 180);         // outline
    const ImU32 colEdge = IM_COL32(64, 160, 255, 220);   // blue
    const ImU32 colText = IM_COL32(180, 180, 180, 255);  // grey
    const float rOuter = 5.0f * m_Dpi;
    const float rInner = 2.0f * m_Dpi;
    const float thick = 2.0f * m_Dpi;

    int sx, sy;
    float ndcDepth;
    ImVec2 sp;
    switch (m_Hover.kind) {
        case HoverKind::Vertex:
            if (worldToScreen(m_Hover.pos, sx, sy, ndcDepth)) {
                // ring highlight
                dl->AddCircle(ImVec2(sx, sy), rOuter, colPt2, 24, thick);
                dl->AddCircle(ImVec2(sx, sy), rOuter - thick * 0.5f, colPt, 24, thick);
                // center dot
                dl->AddCircleFilled(sp, rInner, colPt, 16);

                const float pad = 8.f * m_Dpi;
                ImVec2 origin(pad, m_ViewportHeight - 40.f * m_Dpi);
                char buf[128];
                snprintf(buf, sizeof(buf), "Coords: %.02f %.02f %.02f", m_Hover.pos.x, m_Hover.pos.y, m_Hover.pos.z);
                dl->AddText(origin, colText, buf);
            }
            break;

        case HoverKind::Edge:
            if (m_Hover.edge) {
                ImVec2 a2, b2;
                bool okA = worldToScreen(m_Hover.edge->a, sx, sy, ndcDepth);
                a2 = ImVec2(sx, sy);
                bool okB = worldToScreen(m_Hover.edge->b, sx, sy, ndcDepth);
                b2 = ImVec2(sx, sy);
                if (okA && okB) {
                    dl->AddLine(a2, b2, colEdge, thick);
                    // also mark the nearest point on edge:
                    if (worldToScreen(m_Hover.pos, sx, sy, ndcDepth)) {
                        sp = ImVec2(sx, sy);
                        dl->AddCircleFilled(sp, rInner + 1.0f * m_Dpi, colEdge, 16);
                        dl->AddCircle(sp, rOuter * 0.8f, colEdge, 24, 1.5f * px);
                    }
                }
            }
            break;

        case HoverKind::None:
        default:
            break;
    }
}

}  // namespace pure
