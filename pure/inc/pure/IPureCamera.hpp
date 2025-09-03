#pragma once
#include <glm/glm.hpp>

#include "pure/PureBounds.hpp"

/**
 * @brief Minimal camera interface used by the renderer / controller.
 *
 * Implementations may orbit around a target (dolly), pan the target,
 * zoom/scale the view via scroll, and compute view/projection matrices.
 */
class IPureCamera {
   public:
    virtual ~IPureCamera() = default;

    /// Set the render surface aspect ratio (width / height).
    virtual void SetAspect(float aspect) = 0;

    /// Fit the current scene bounds into view. `padding` > 1 adds some margin.
    virtual void FitToBounds(const PureBounds& bounds, float padding = 1.1f) = 0;

    /// Mouse wheel / trackpad scroll. Positive usually means zoom-in.
    virtual void OnScrollWheel(float delta) = 0;

    /// Orbit the camera around the target (delta given in pixels or abstract units).
    virtual void Orbit(float dx, float dy) = 0;

    /// Pan the target along the image plane (delta given in pixels or abstract units).
    virtual void Pan(float dx, float dy) = 0;

    /// Get the 4×4 view matrix (world → view).
    virtual glm::mat4 View() const = 0;

    /// Get the 4×4 projection matrix (view → clip).
    virtual glm::mat4 Projection() const = 0;

    /// Current eye position in world space.
    virtual glm::vec3 Position() const = 0;

    /// Normalized view direction (from eye to target).
    virtual glm::vec3 ViewDirection() const = 0;
};
