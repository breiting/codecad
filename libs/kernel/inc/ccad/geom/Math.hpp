#pragma once

namespace ccad {

/** \brief Lightweight 3D vector (double precision) for the public API. */
struct Vec3 {
    double x{0}, y{0}, z{0};
};

/** \brief Axis-aligned bounding box in model units. */
struct Bounds {
    Vec3 min{0, 0, 0};
    Vec3 max{0, 0, 0};

    /// True if min<=max on all axes
    bool valid() const {
        return (min.x <= max.x) && (min.y <= max.y) && (min.z <= max.z);
    }

    /// Box center
    Vec3 center() const {
        return {(min.x + max.x) * 0.5, (min.y + max.y) * 0.5, (min.z + max.z) * 0.5};
    }

    /// Box size (max - min), not clamped.
    Vec3 size() const {
        return {max.x - min.x, max.y - min.y, max.z - min.z};
    }
};

/** \brief Rigid transform with uniform scale, Euler rotations in degrees. */
struct Transform {
    Vec3 translation{0, 0, 0};
    double rotX{0}, rotY{0}, rotZ{0};  ///< degrees
    double scale{1.0};
};

}  // namespace ccad
