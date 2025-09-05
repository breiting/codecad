#pragma once
#include <ccad/base/Math.hpp>
#include <ccad/base/Shape.hpp>
#include <vector>

namespace ccad {
namespace draft {

/// Planar section of a solid with a plane, which returns a list of polylines.
/// @param s     Solid shape
/// @param plane 3D plane
/// @param defl  Max deflection for curve discretization (mm)
std::vector<Polyline2D> SectionWithPlane(const Shape& s, const Plane3& plane, double defl = 0.2);
}  // namespace draft
}  // namespace ccad
