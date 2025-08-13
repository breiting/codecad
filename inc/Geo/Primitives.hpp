#pragma once
#include "geo/Shape.hpp"

namespace geo {

// Create an axis-aligned box with given dimensions (in mm).
ShapePtr MakeBox(double x, double y, double z);

// Create a cylinder (in mm).
ShapePtr MakeCylinder(double diameter, double height);

// Hex-Prisma (across-flats = Schlüsselweite SW)
ShapePtr MakeHexPrism(double across_flats, double height);

}  // namespace geo
