#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <optional>
#include <pure/PureTypes.hpp>

namespace pure {

enum class PickKind { Point, Edge };

struct PickPoint {
    glm::vec3 p;
};
struct PickEdge {
    Edge e;
};

using Pick = std::variant<PickPoint, PickEdge>;

enum class MeasureKind { P2P, P2E, E2E };

struct MeasureResult {
    MeasureKind kind;
    float distance;
    std::string label;  // e.g. "P↔E: 12.34 mm"
};

/**
 * @brief Minimal tool that handles a 2-click measurement workflow
 */
class PureMeasurement {
   public:
    PureMeasurement();
    // Callback für UI
    using Reporter = std::function<void(const std::string&)>;

    void SetReporter(Reporter r) {
        m_Reporter = std::move(r);
    }

    bool Enabled() const {
        return m_Enabled;
    }

    void Enable(bool on);

    // Will be called by controller for every pick
    void OnPick(const Pick& pick);

   private:
    void Reset();
    std::optional<MeasureResult> Measure(const Pick& a, const Pick& b);
    std::string Format(const char* f, float v);

   private:
    bool m_Enabled{false};
    std::optional<Pick> m_First;
    Reporter m_Reporter;
};

}  // namespace pure
