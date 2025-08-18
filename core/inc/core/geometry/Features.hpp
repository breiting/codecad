/// \file
/// Geometry API for solid modeling (OCCT wrappers).
#pragma once
#include "geometry/Shape.hpp"

namespace geometry {

// Round-off edges with constant radius (mm)
/// \brief FilletAll — see docs/LUA_API.md for typical usage.
ShapePtr FilletAll(const ShapePtr& s, double radiusMm);

// Apply a chamfer on all edges with the given distance
/// \brief ChamferAll — see docs/LUA_API.md for typical usage.
ShapePtr ChamferAll(const ShapePtr& s, double distanceMm);

}  // namespace geometry
