#pragma once

#include <ostream>
namespace ccad {

constexpr double DegToRad(double d) {
    return d * M_PI / 180.0;
}

/** \brief Lightweight 2D vector (double precision) for the public API. */
class Vec2 {
   public:
    Vec2() = default;
    Vec2(double x, double y) : x(x), y(y) {
    }
    double x{0}, y{0};

    friend std::ostream& operator<<(std::ostream& os, const Vec2& v) {
        os << "Vec2(" << v.x << ", " << v.y << ")";
        return os;
    }
};

/** \brief Lightweight 3D vector (double precision) for the public API. */
class Vec3 {
   public:
    Vec3() = default;
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {
    }
    double x{0}, y{0}, z{0};

    friend std::ostream& operator<<(std::ostream& os, const Vec3& v) {
        os << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }
};

/** \brief A simple plane 3D definition */
class Plane3 {
   public:
    Vec3 point;   ///< any point on the plane
    Vec3 normal;  ///< normalized normal vector
};

/** \brief Simple polyline datastructure */
template <typename T>
class Polyline {
   public:
    std::vector<T> points;
};

using Polyline2D = Polyline<Vec2>;
using Polyline3D = Polyline<Vec3>;

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

static inline double Clamp(double v, double lo, double hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

static inline double Lerp(double a, double b, double s) {
    return a + (b - a) * s;
}

}  // namespace ccad
