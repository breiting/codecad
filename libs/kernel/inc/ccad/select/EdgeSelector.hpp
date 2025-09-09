#pragma once
/**
 * @file EdgeSelector.hpp
 * @brief Query-style edge selection for fillet/chamfer and analysis.
 *
 * Public API is OCCT-free. Internally, the implementation builds a fast
 * per-edge cache (type, length, radius, an approximate direction, a sample point,
 * dihedral angle if planar faces are adjacent, etc.) and then applies chainable
 * filters. This keeps interactive use fast and code maintainable.
 */

#include <ccad/base/Math.hpp>
#include <ccad/base/Shape.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace ccad::select {

/// Geometric kind of an edge (as seen by the selector).
enum class EdgeGeom { Any, Line, Circle, Arc };

/// Axis-aligned bounding-box sides for convenience filters.
enum class BoxSide { XMin, XMax, YMin, YMax, ZMin, ZMax };

/// Range helpers for filters.
struct AngleRange {
    double minDeg = 0.0, maxDeg = 180.0;
};
struct LengthRange {
    double min = 0.0, max = 1e100;
};
struct RadiusRange {
    double min = 0.0, max = 1e100;
};

/**
 * @brief Lightweight, stable description of an edge for downstream use.
 *
 * This is safe to pass around in the kernel and UI; it contains no OCCT types.
 * Use `index` as a stable identifier within the originating EdgeSelector.
 */
struct EdgeRef {
    size_t index = 0;            ///< Stable index within the selector’s internal table
    glm::vec3 approxDir{};       ///< Tangent direction at mid-parameter (normalized)
    glm::vec3 anyPoint{};        ///< A sample 3D point on the edge (world coordinates)
    double length = 0.0;         ///< Edge length
    bool isCircular = false;     ///< True for circle/arc
    double radius = 0.0;         ///< Circle/arc radius (if applicable)
    double dihedralDeg = 180.0;  ///< Angle between adjacent planar faces; 180≈smooth, small≈sharp
};

/**
 * @brief A small container for selected edges with set-algebra convenience.
 */
class EdgeSet {
   public:
    const std::vector<EdgeRef>& items() const {
        return m_items;
    }
    bool empty() const {
        return m_items.empty();
    }
    size_t size() const {
        return m_items.size();
    }

    /// Union with another set (in-place). Index is the identity.
    EdgeSet& unite(const EdgeSet& b);
    /// Intersection with another set (in-place).
    EdgeSet& intersect(const EdgeSet& b);
    /// Subtract another set (in-place).
    EdgeSet& subtract(const EdgeSet& b);

   private:
    std::vector<EdgeRef> m_items;
    friend class EdgeSelector;  // so selector can fill items efficiently
};

/**
 * @brief Chainable query builder over the edges of a Shape.
 *
 * Typical usage:
 * @code
 * using namespace ccad::select;
 * auto edges = EdgeSelector::FromShape(shape)
 *                .onBoxSide(BoxSide::ZMin, 1e-4)
 *                .geom(EdgeGeom::Line)
 *                .parallelTo(Axis::X, 5.0)
 *                .collect();
 * // features::Chamfer(shape, edges.items(), 0.5);
 * @endcode
 */
class EdgeSelector {
   public:
    /// Build a selector from a Shape. A cache of edge attributes is computed once.
    static EdgeSelector FromShape(const ccad::Shape& shape, double tolerance = 1e-6);

    // -------- chainable filters (each returns *this) --------

    /// Keep edges of a given geometric kind (line/arc/circle) or anything.
    EdgeSelector& geom(EdgeGeom g);

    /// Keep edges with length within [min, max].
    EdgeSelector& lengthBetween(const LengthRange& r);

    /// Keep circular/arc edges with radius within [min, max].
    EdgeSelector& radiusBetween(const RadiusRange& r);

    /// Keep edges whose dihedral (between adjacent planar faces) lies within [minDeg, maxDeg].
    EdgeSelector& dihedralBetween(const AngleRange& r);

    /// Keep edges roughly parallel to an axis (within tolerance in degrees).
    EdgeSelector& parallelTo(Axis axis, double tolDeg = 5.0);

    /// Keep edges whose sample point lies near a plane (axis=value) within absolute tolerance.
    EdgeSelector& nearPlane(Axis axis, double value, double tol = 1e-6);

    /// Keep edges located on a specific AABB side of the *shape’s* bounding box (with tolerance).
    EdgeSelector& onBoxSide(BoxSide side, double tol = 1e-6);

    /// Keep edges whose sample point lies inside the given axis-aligned box.
    EdgeSelector& insideAABB(const glm::vec3& minP, const glm::vec3& maxP);

    // -------- execute --------

    /// Evaluate the filters and return the resulting set.
    EdgeSet collect() const;

   private:
    struct Impl;                   // PIMPL to hide OCCT
    std::shared_ptr<Impl> m_impl;  // shared so copies are cheap
    explicit EdgeSelector(std::shared_ptr<Impl> impl) : m_impl(std::move(impl)) {
    }

    // filter state
    struct Filters {
        std::optional<EdgeGeom> geom;
        std::optional<LengthRange> length;
        std::optional<RadiusRange> radius;
        std::optional<AngleRange> dihedral;
        std::optional<std::pair<Axis, double>> parallelAxisTolDeg;
        std::optional<std::tuple<Axis, double, double>> planeAxisValTol;
        std::optional<std::pair<BoxSide, double>> boxSideTol;
        std::optional<std::pair<glm::vec3, glm::vec3>> aabb;
    };
    Filters m_filters{};
};

}  // namespace ccad::select
