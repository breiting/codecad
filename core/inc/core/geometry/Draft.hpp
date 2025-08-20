#pragma once
#include <utility>
#include <vector>

#include "geometry/Shape.hpp"

namespace geometry {

using Polyline2D = std::vector<std::pair<double, double>>;

/// Axis-aligned planar section of a solid (X=c, Y=c, or Z=c),
/// discretized to 2D polylines in the plane's local axes:
///  - axis='z' -> (x,y)
///  - axis='x' -> (y,z)
///  - axis='y' -> (x,z)
/// @param s     Solid shape
/// @param axis  'x', 'y', or 'z'
/// @param value Plane offset along axis (in mm)
/// @param defl  Max deflection for curve discretization (mm)
std::vector<Polyline2D> SectionOutline2D(const ShapePtr& s, char axis, double value, double defl = 0.2);

}  // namespace geometry
