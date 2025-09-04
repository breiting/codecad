#pragma once

#include <ostream>
namespace ccad {

constexpr double DegToRad(double d) {
    return d * M_PI / 180.0;
}

/** \brief Lightweight 2D vector (double precision) for the public API. */
struct Vec2 {
    double x{0}, y{0};

    friend std::ostream& operator<<(std::ostream& os, const Vec2& v) {
        os << "Vec3(" << v.x << ", " << v.y << ")";
        return os;
    }
};

/** \brief Lightweight 3D vector (double precision) for the public API. */
struct Vec3 {
    double x{0}, y{0}, z{0};

    friend std::ostream& operator<<(std::ostream& os, const Vec3& v) {
        os << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }
};

/** \brief Axis-aligned bounding box in model units. */
class Bounds {
   public:
    Vec3 min{0, 0, 0};
    Vec3 max{0, 0, 0};

    /// True if min<=max on all axes
    bool IsValid() const {
        return (min.x <= max.x) && (min.y <= max.y) && (min.z <= max.z);
    }

    /// Box center
    Vec3 Center() const {
        return {(min.x + max.x) * 0.5, (min.y + max.y) * 0.5, (min.z + max.z) * 0.5};
    }

    /// Box size (max - min), not clamped.
    Vec3 Size() const {
        return {max.x - min.x, max.y - min.y, max.z - min.z};
    }

    friend std::ostream& operator<<(std::ostream& os, const Bounds& b) {
        os << "Bounds(min=(" << b.min.x << ", " << b.min.y << ", " << b.min.z << "), max=(" << b.max.x << ", "
           << b.max.y << ", " << b.max.z << "))";
        return os;
    }
};

/** \brief Rigid transform with uniform scale, Euler rotations in degrees. */
struct Transform {
    Vec3 translation{0, 0, 0};
    double rotX{0}, rotY{0}, rotZ{0};  ///< degrees
    double scale{1.0};

    friend std::ostream& operator<<(std::ostream& os, const Transform& t) {
        os << "Transform(translation=(" << t.translation.x << ", " << t.translation.y << ", " << t.translation.z
           << "), "
           << "rotation=(" << t.rotX << "°, " << t.rotY << "°, " << t.rotZ << "°), "
           << "scale=" << t.scale << ")";
        return os;
    }
};

}  // namespace ccad
