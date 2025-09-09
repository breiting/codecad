#pragma once
#include <imgui.h>

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <optional>
#include <pure/IPurePickProvider.hpp>
#include <pure/PureScene.hpp>
#include <vector>

namespace pure {

class PurePicker : public IPurePickProvider {
   public:
    enum class HoverKind { None, Vertex, Edge };

    struct HoverState {
        HoverKind kind{HoverKind::None};
        glm::vec3 pos{0};
        std::optional<Edge> edge;
    };

    void UpdateHover(float mouseX, float mouseY);

    void DrawHoverOverlay(ImDrawList* dl) const;

    const HoverState& Hover() const {
        return m_Hover;
    }

    void SetScene(const PureScene* scene) {
        m_Scene = scene;
    }
    void SetViewProj(const glm::mat4& view, const glm::mat4& proj) {
        m_View = view;
        m_Proj = proj;
        m_ViewProj = proj * view;
        m_InvViewProj = glm::inverse(m_ViewProj);
        m_CamPos = glm::vec3(glm::inverse(m_View)[3]);
        // rough forward from view (camera looks along -Z in view space)
        m_CamForward = glm::normalize(glm::vec3(glm::inverse(m_View) * glm::vec4(0, 0, -1, 0)));
    }
    void SetViewport(int w, int h, float dpi = 1.0f) {
        m_ViewportWidth = w;
        m_ViewportHeight = h;
        m_Dpi = dpi;
    }

    void SetSnapPixels(float px) {
        m_SnapPx = px;
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

    bool worldToScreen(const glm::vec3& w, int& outX, int& outY, float& outDepth01) const;

    void screenRay(float mx, float my, glm::vec3& ro, glm::vec3& rd) const;
    static bool rayTriangle(const glm::vec3& ro, const glm::vec3& rd, const glm::vec3& a, const glm::vec3& b,
                            const glm::vec3& c, float& t, float& u, float& v);

    float pixelToWorld(float px, float depthWorld) const;

    const EdgeCache& getEdgeCache(const PureMesh* m);

    // snapping
    bool snapVertex(const glm::vec3& ro, const glm::vec3& rd, glm::vec3& outHitPos);
    bool snapEdge(const glm::vec3& ro, const glm::vec3& rd, glm::vec3& outHitPos, std::optional<Edge>& edge);
    bool hitFace(const glm::vec3& ro, const glm::vec3& rd, glm::vec3& outHitPos);

    bool isVisibleWorldPoint(const glm::vec3& w) const;
    bool sampleDepth(int sx, int sy, float& outDepth) const;

   private:
    const PureScene* m_Scene{nullptr};
    glm::mat4 m_View{1}, m_Proj{1}, m_ViewProj{1}, m_InvViewProj{1};
    glm::vec3 m_CamPos{0}, m_CamForward{0, 0, -1};
    int m_ViewportWidth{1}, m_ViewportHeight{1};
    float m_Dpi{1.0f};
    float m_SnapPx{8.0f};
    HoverState m_Hover;

    mutable std::vector<EdgeCache> m_edgeCaches;
};

}  // namespace pure
