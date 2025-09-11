#pragma once
#include <glm/glm.hpp>

#include "pure/PureBounds.hpp"

/**
 * @brief Camera bookmark which can be used to store a camera setup
 */
struct CameraBookmark {
    glm::vec3 position{};
    glm::vec3 target{};
    glm::vec3 up{0, 0, 1};
    float fovDeg{45.f};
};

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

    /// Normalized view direction (from eye to target).
    virtual glm::vec3 ViewDirection() const = 0;

    /// Get current eye position in world space.
    virtual glm::vec3 Position() const = 0;

    /// Get target (lookAt)
    virtual glm::vec3 Target() const = 0;

    /// Get the field of view in degrees
    virtual float FovDeg() const = 0;

    /// Set current eye position in world space.
    virtual void SetPosition(const glm::vec3&) = 0;

    /// Set target (lookAt)
    virtual void SetTarget(const glm::vec3&) = 0;

    /// Set the field of view in degrees
    virtual void SetFovDeg(float) = 0;
};
