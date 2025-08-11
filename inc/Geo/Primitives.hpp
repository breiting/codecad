#pragma once
#include "Geo/Shape.hpp"

namespace Geo {

// Create an axis-aligned box with given dimensions (in mm).
ShapePtr MakeBox(double x, double y, double z);

// Create a cylinder (in mm).
ShapePtr MakeCylinder(double r, double h);

}  // namespace Geo
