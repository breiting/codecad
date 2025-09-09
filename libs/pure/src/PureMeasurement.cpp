#include <pure/PureMath.hpp>
#include <pure/PureMeasurement.hpp>

using namespace pure::math;
namespace pure {

PureMeasurement::PureMeasurement() = default;

void PureMeasurement::Enable(bool on) {
    m_Enabled = on;
    Reset();
    if (m_Reporter) m_Reporter(on ? "Measure: ON" : "Measure: OFF");
}

void PureMeasurement::Reset() {
    m_First.reset();
}

void PureMeasurement::OnPick(const Pick& pick) {
    if (!m_Enabled) return;
    if (!m_First) {
        m_First = pick;
        if (m_Reporter) m_Reporter("1st picked. Pick 2nd ...");
        return;
    }
    auto res = Measure(*m_First, pick);
    if (res && m_Reporter) m_Reporter(res->label);
    Reset();
}

std::optional<MeasureResult> PureMeasurement::Measure(const Pick& a, const Pick& b) {
    if (std::holds_alternative<PickPoint>(a) && std::holds_alternative<PickPoint>(b)) {
        auto p1 = std::get<PickPoint>(a).p;
        auto p2 = std::get<PickPoint>(b).p;
        float d = math::DistancePointPoint(p1, p2);
        return MeasureResult{MeasureKind::P2P, d, Format("Distance (P-P): %.3f mm", d)};
    }
    if (std::holds_alternative<PickPoint>(a) && std::holds_alternative<PickEdge>(b)) {
        auto p = std::get<PickPoint>(a).p;
        auto e = std::get<PickEdge>(b).e;
        float d = math::DistancePointEdge(p, e);
        return MeasureResult{MeasureKind::P2E, d, Format("Distance (P-E): %.3f mm", d)};
    }
    if (std::holds_alternative<PickEdge>(a) && std::holds_alternative<PickPoint>(b)) {
        auto e = std::get<PickEdge>(a).e;
        auto p = std::get<PickPoint>(b).p;
        float d = math::DistancePointEdge(p, e);
        return MeasureResult{MeasureKind::P2E, d, Format("Distance (P-E): %.3f mm", d)};
    }
    if (std::holds_alternative<PickEdge>(a) && std::holds_alternative<PickEdge>(b)) {
        auto e1 = std::get<PickEdge>(a).e;
        auto e2 = std::get<PickEdge>(b).e;
        float d = math::DistanceEdgeEdgeSimple(e1, e2, 5.0f);
        return MeasureResult{MeasureKind::E2E, d, Format("Distance (E-E): %.3f mm", d)};
    }
    return std::nullopt;
}

std::string PureMeasurement::Format(const char* f, float v) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), f, v);
    return std::string(buf);
}
}  // namespace pure
