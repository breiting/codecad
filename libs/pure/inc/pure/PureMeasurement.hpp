#pragma once
#include <imgui.h>

#include <glm/glm.hpp>
#include <optional>
#include <pure/IPurePickProvider.hpp>
#include <pure/PureTypes.hpp>

namespace pure {

struct MeasurementResult {
    MeasureMode mode{MeasureMode::PointToPoint};
    ConstraintAxis constraint{ConstraintAxis::None};
    // Primary result
    float distance{0.f};
    glm::vec3 p0{0}, p1{0};  // measured points (or closest pair for edge/edge)
    // Optional extra for Edge→Edge
    std::optional<Edge> s0;  // original picked edge A
    std::optional<Edge> s1;  // original picked edge B
};

/**
 * @brief Minimal tool that handles a 2-click measurement workflow with optional axis constraint.
 *
 * Usage:
 *  - Create once, inject IPickProvider + viewport size setter.
 *  - On input: call OnMouseMove / OnMouseButton / OnKey
 *  - Per frame: call DrawOverlay(ImDrawList*), QueryResult()
 */
class PureMeasurement {
   public:
    PureMeasurement();

    void SetPicker(IPurePickProvider* picker) {
        m_Picker = picker;
    }
    void SetMode(MeasureMode m) {
        m_Mode = m;
        Reset();
    }
    void SetConstraint(ConstraintAxis a) {
        m_Constraint = a;
    }
    ConstraintAxis Constraint() const {
        return m_Constraint;
    }
    MeasureMode Mode() const {
        return m_Mode;
    }

    // Input hooks (call from your controller)
    void OnMouseMove(float x, float y);
    void OnMouseButton(int button, bool pressed, bool shiftHeld);
    void OnKey(int key, bool pressed);

    /// Returns a result if a complete measurement exists
    const std::optional<MeasurementResult>& Result() const {
        return m_Result;
    }

    /// Clears current picks and result
    void Reset();

    /// UI overlay (ImGui drawlist for screen-space)
    void DrawOverlay(ImDrawList* dl) const;

    void SetViewport(int w, int h, float dpi = 1.0f) {
        m_ViewportWidth = w;
        m_ViewportHeight = h;
        m_Dpi = dpi;
    }

   private:
    // Internal helpers
    void TryPickCurrent();
    void ComputePointToPoint();
    void ComputeEdgeToEdge();

   private:
    IPurePickProvider* m_Picker{nullptr};

    MeasureMode m_Mode{MeasureMode::PointToPoint};
    ConstraintAxis m_Constraint{ConstraintAxis::None};

    // Mouse state
    glm::vec2 m_Mouse{0};

    // Picks
    bool m_FirstFixed{false};    // first click fixed
    glm::vec3 m_P0{0}, m_P1{0};  // current measured points
    std::optional<Edge> m_E0;    // first picked segment (edge mode)
    std::optional<Edge> m_E1;    // second picked segment (edge mode)

    // Live hover (second point pending)
    glm::vec3 m_HoverP{0};
    std::optional<Edge> m_HoverEdge;
    int m_ViewportWidth{1}, m_ViewportHeight{1};
    float m_Dpi{1.0f};

    std::optional<MeasurementResult> m_Result;
};

}  // namespace pure
