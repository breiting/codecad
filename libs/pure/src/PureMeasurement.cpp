#include <imgui.h>  // for ImDrawList

#include <algorithm>
#include <cmath>
#include <pure/PureMath.hpp>
#include <pure/PureMeasurement.hpp>

using namespace pure::math;
namespace pure {

PureMeasurement::PureMeasurement() = default;

void PureMeasurement::NewFrame() {
    // reset live hover each frame; recomputed in TryPickCurrent
    m_HoverEdge.reset();
}

void PureMeasurement::EndFrame() {
    // nothing
}

void PureMeasurement::Reset() {
    m_FirstFixed = false;
    m_E0.reset();
    m_E1.reset();
    m_Result.reset();
}

void PureMeasurement::OnMouseMove(float x, float y) {
    m_Mouse = {x, y};
    TryPickCurrent();
    if (m_FirstFixed) {
        if (m_Mode == MeasureMode::PointToPoint) {
            m_P1 = Constrain(m_P0, m_HoverP, m_Constraint);
            ComputePointToPoint();
        } else {
            // edge mode: we have m_E0 fixed, update hover edge to E1, then compute
            if (m_HoverEdge) {
                m_E1 = m_HoverEdge;
                ComputeEdgeToEdge();
            } else {
                m_Result.reset();
            }
        }
    }
}

void PureMeasurement::OnMouseButton(int button, bool pressed, bool shiftHeld) {
    if (!m_Picker) return;
    if (button != 0 || !pressed) return;  // left down only

    if (!m_FirstFixed) {
        // Fix first
        if (m_Mode == MeasureMode::PointToPoint) {
            // pick point (prefer vertex, then edge, then face)
            glm::vec3 hp;
            std::optional<Edge> tmp;
            if (m_Picker->Pick(m_Mouse.x, m_Mouse.y, SnapType::Vertex, hp, tmp) ||
                m_Picker->Pick(m_Mouse.x, m_Mouse.y, SnapType::Edge, hp, tmp) ||
                m_Picker->Pick(m_Mouse.x, m_Mouse.y, SnapType::Face, hp, tmp)) {
                m_P0 = hp;
                m_FirstFixed = true;
            }
        } else {
            // pick edge (prefer edge snap; fallback: derive small edge from face hit direction)
            glm::vec3 hp;
            std::optional<Edge> seg;
            if (m_Picker->Pick(m_Mouse.x, m_Mouse.y, SnapType::Edge, hp, seg)) {
                m_E0 = seg;
                m_FirstFixed = true;
            }
        }
    } else {
        // Fix second
        if (m_Mode == MeasureMode::PointToPoint) {
            glm::vec3 hp;
            std::optional<Edge> tmp;
            if (m_Picker->Pick(m_Mouse.x, m_Mouse.y, SnapType::Vertex, hp, tmp) ||
                m_Picker->Pick(m_Mouse.x, m_Mouse.y, SnapType::Edge, hp, tmp) ||
                m_Picker->Pick(m_Mouse.x, m_Mouse.y, SnapType::Face, hp, tmp)) {
                m_P1 = Constrain(m_P0, hp, m_Constraint);
                ComputePointToPoint();
                m_FirstFixed = false;  // ready for next measure
            }
        } else {
            glm::vec3 hp;
            std::optional<Edge> seg;
            if (m_Picker->Pick(m_Mouse.x, m_Mouse.y, SnapType::Edge, hp, seg)) {
                m_E1 = seg;
                ComputeEdgeToEdge();
                m_FirstFixed = false;
            }
        }
    }
}

void PureMeasurement::OnKey(int key, bool pressed) {
    if (!pressed) return;
    // Minimal default bindings; wire to your controller keys
    // X/Y/Z set constraint; C clears constraint; ESC resets session
    switch (key) {
        case 'X':
            SetConstraint(ConstraintAxis::X);
            break;
        case 'Y':
            SetConstraint(ConstraintAxis::Y);
            break;
        case 'Z':
            SetConstraint(ConstraintAxis::Z);
            break;
        case 'C':
            SetConstraint(ConstraintAxis::None);
            break;
        case 27:
            Reset();
            break;  // ESC
        default:
            break;
    }
}

void PureMeasurement::TryPickCurrent() {
    if (!m_Picker) return;
    glm::vec3 hp;
    std::optional<Edge> seg;

    if (m_Mode == MeasureMode::PointToPoint) {
        if (m_Picker->Pick(m_Mouse.x, m_Mouse.y, SnapType::Vertex, hp, seg) ||
            m_Picker->Pick(m_Mouse.x, m_Mouse.y, SnapType::Edge, hp, seg) ||
            m_Picker->Pick(m_Mouse.x, m_Mouse.y, SnapType::Face, hp, seg)) {
            m_HoverP = hp;
        }
    } else {
        if (m_Picker->Pick(m_Mouse.x, m_Mouse.y, SnapType::Edge, hp, seg)) {
            m_HoverP = hp;
            m_HoverEdge = seg;
        }
    }
}

void PureMeasurement::ComputePointToPoint() {
    glm::vec3 q1 = m_P1;
    float d = glm::length(q1 - m_P0);
    if (m_Constraint != ConstraintAxis::None) {
        d = std::abs(ProjectedLength(q1 - m_P0, m_Constraint));
    }
    MeasurementResult r;
    r.mode = MeasureMode::PointToPoint;
    r.constraint = m_Constraint;
    r.p0 = m_P0;
    r.p1 = q1;
    r.distance = d;
    m_Result = r;
}

void PureMeasurement::ComputeEdgeToEdge() {
    if (!m_E0 || !m_E1) {
        m_Result.reset();
        return;
    }
    glm::vec3 c0, c1;
    float d2 = ClosestPointsOnSegments(*m_E0, *m_E1, c0, c1);
    float d = std::sqrt(std::max(0.f, d2));

    if (m_Constraint != ConstraintAxis::None) {
        d = std::abs(ProjectedLength(c1 - c0, m_Constraint));
    }

    MeasurementResult r;
    r.mode = MeasureMode::EdgeToEdge;
    r.constraint = m_Constraint;
    r.p0 = c0;
    r.p1 = c1;
    r.distance = d;
    r.s0 = m_E0;
    r.s1 = m_E1;
    m_Result = r;
}

static ImU32 U32RGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
    return IM_COL32(r, g, b, a);
}

void PureMeasurement::DrawOverlay(void* drawListVoid, const glm::mat4& /*viewProj*/, const glm::vec2& viewportPx,
                                  float dpiScale) const {
    ImDrawList* dl = reinterpret_cast<ImDrawList*>(drawListVoid);
    if (!dl) return;

    // HUD (bottom-left)
    const float pad = 8.f * dpiScale;
    ImVec2 origin(pad, viewportPx.y - 60.f * dpiScale);
    char buf[128];
    if (m_Result) {
        snprintf(buf, sizeof(buf), "dist: %.3f mm%s", m_Result->distance,
                 (m_Result->constraint == ConstraintAxis::None ? ""
                  : m_Result->constraint == ConstraintAxis::X  ? " (X)"
                  : m_Result->constraint == ConstraintAxis::Y  ? " (Y)"
                                                               : " (Z)"));
        dl->AddText(origin, U32RGBA(255, 255, 255), buf);
    } else {
        const char* help = (m_Mode == MeasureMode::PointToPoint) ? "Measure P-to-P: click first point, click second"
                                                                 : "Measure E-to-E: click first edge, click second";
        dl->AddText(origin, U32RGBA(180, 180, 180), help);
    }

    // In a minimal variant we skip projecting world→screen here and rely on your existing
    // gizmo/overlay if you have one. If you want, you can add a tiny line between p0/p1
    // in screen space using your world->screen helper.
}

}  // namespace pure
