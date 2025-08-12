#pragma once
#include <vector>

#include "geo/Shape.hpp"

namespace geo {

// Boolean union (fuse) of multiple shapes. Expects at least 2 shapes.
ShapePtr MakeUnion(const std::vector<ShapePtr>& shapes);

// Boolean difference: a - b
ShapePtr MakeDifference(const ShapePtr& a, const ShapePtr& b);

// Intersection
ShapePtr MakeIntersect(const ShapePtr& a, const ShapePtr& b);

}  // namespace geo
