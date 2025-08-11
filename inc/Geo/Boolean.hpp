#pragma once
#include <vector>

#include "Geo/Shape.hpp"

namespace Geo {

// Boolean union (fuse) of multiple shapes. Expects at least 2 shapes.
ShapePtr MakeUnion(const std::vector<ShapePtr>& shapes);

}  // namespace Geo
