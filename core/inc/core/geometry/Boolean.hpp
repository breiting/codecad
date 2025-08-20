/// \file
/// Geometry API for solid modeling (OCCT wrappers).
#pragma once
#include <vector>

#include "geometry/Shape.hpp"

namespace geometry {

// Boolean union (fuse) of multiple shapes. Expects at least 2 shapes.
/// \brief MakeUnion — see docs/LUA_API.md for typical usage.
ShapePtr MakeUnion(const std::vector<ShapePtr>& shapes);

// Boolean difference: a - b
/// \brief MakeDifference — see docs/LUA_API.md for typical usage.
ShapePtr MakeDifference(const ShapePtr& a, const ShapePtr& b);

// Intersection
/// \brief MakeIntersect — see docs/LUA_API.md for typical usage.
ShapePtr MakeIntersect(const ShapePtr& a, const ShapePtr& b);

}  // namespace geometry