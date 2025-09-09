#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <optional>
#include <pure/IPurePicker.hpp>
#include <pure/PureScene.hpp>
#include <vector>

namespace pure {

class PurePicker : public IPurePicker {
   public:
    // Scene / camera wiring
    void SetScene(const PureScene* scene) {
        m_scene = scene;
    }
    void SetViewProj(const glm::mat4& view, const glm::mat4& proj) {
        m_view = view;
        m_proj = proj;
        m_viewProj = proj * view;
        m_invViewProj = glm::inverse(m_viewProj);
        m_camPos = glm::vec3(glm::inverse(m_view)[3]);
        // rough forward from view (camera looks along -Z in view space)
        m_camForward = glm::normalize(glm::vec3(glm::inverse(m_view) * glm::vec4(0, 0, -1, 0)));
    }
    void SetViewport(int w, int h, float dpi = 1.0f) {
        m_vpW = w;
        m_vpH = h;
        m_dpi = dpi;
    }

    // tuning
    void SetSnapPixels(float px) {
        m_snapPx = px;
    }

    // IPickProvider
    bool Pick(float mouseX, float mouseY, SnapType snap, glm::vec3& outHitPos, std::optional<Edge>& outEdge) override;

   private:
    struct EdgeIds {
        uint32_t i0, i1;
    };
    struct EdgeCache {  // built once per mesh
        const PureMesh* mesh{nullptr};
        std::vector<EdgeIds> edges;
    };

    // helpers
    void screenRay(float mx, float my, glm::vec3& ro, glm::vec3& rd) const;
    static bool rayTriangle(const glm::vec3& ro, const glm::vec3& rd, const glm::vec3& a, const glm::vec3& b,
                            const glm::vec3& c, float& t, float& u, float& v);

    // pixel → world tolerance an Tiefe z
    float pixelToWorld(float px, float depthWorld) const;

    // caches
    const EdgeCache& getEdgeCache(const PureMesh* m);

    // snapping
    bool snapVertex(const glm::vec3& ro, const glm::vec3& rd, glm::vec3& outHitPos);
    bool snapEdge(const glm::vec3& ro, const glm::vec3& rd, glm::vec3& outHitPos, std::optional<Edge>& edge);
    bool hitFace(const glm::vec3& ro, const glm::vec3& rd, glm::vec3& outHitPos);

   private:
    const PureScene* m_scene{nullptr};
    glm::mat4 m_view{1}, m_proj{1}, m_viewProj{1}, m_invViewProj{1};
    glm::vec3 m_camPos{0}, m_camForward{0, 0, -1};
    int m_vpW{1}, m_vpH{1};
    float m_dpi{1.0f};
    float m_snapPx{8.0f};

    mutable std::vector<EdgeCache> m_edgeCaches;
};

}  // namespace pure
