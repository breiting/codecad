/// \file
/// Geometry API for solid modeling (OCCT wrappers).
#pragma once
#include "geo/Shape.hpp"

namespace geo {

// Create an axis-aligned box with given dimensions (in mm).
/// \brief MakeBox — see docs/LUA_API.md for typical usage.
ShapePtr MakeBox(double x, double y, double z);

// Create a cylinder (in mm).
/// \brief MakeCylinder — see docs/LUA_API.md for typical usage.
ShapePtr MakeCylinder(double diameter, double height);

// Hexgonal Prism (e.g. bolt)
/// \brief MakeHexPrism — see docs/LUA_API.md for typical usage.
ShapePtr MakeHexPrism(double acrossFlats, double height);

}  // namespace geo
