#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <pure/PureTypes.hpp>

namespace pure {
/**
 * @brief Lightweight picking interface PureMeasurement relies on.
 * Implement this in your controller using your existing raycaster.
 */
class IPurePicker {
   public:
    virtual ~IPurePicker() = default;
    /**
     * @param mouseX, mouseY: window coords in pixels
     * @param snap: desired snap type priority
     * @param outHitPos: world-space hit (vertex pos / closest point on edge / face hit)
     * @param outEdge: if snap==Edge and a segment was found, return it (world-space)
     * @return true if something was picked
     */
    virtual bool Pick(float mouseX, float mouseY, SnapType snap, glm::vec3& outHitPos,
                      std::optional<Edge>& outEdge) = 0;
};
}  // namespace pure
