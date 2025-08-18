/// \file
/// Geometry API for solid modeling (OCCT wrappers).
#pragma once
#include "geometry/Shape.hpp"

namespace geometry {

// Create an axis-aligned box with given dimensions (in mm).
/// \brief MakeBox — see docs/LUA_API.md for typical usage.
ShapePtr MakeBox(double x, double y, double z);

// Create a cylinder (in mm).
/// \brief MakeCylinder — see docs/LUA_API.md for typical usage.
ShapePtr MakeCylinder(double diameter, double height);

// Create a sphere (in mm).
/// \brief MakeSphere — see docs/LUA_API.md for typical usage.
ShapePtr MakeSphere(double diameter);

// Create a cone (in mm).
/// \brief MakeCone — see docs/LUA_API.md for typical usage.
ShapePtr MakeCone(double diameter1, double diameter2, double height);

// Create a wedge (in mm).
/// \brief MakeWedge — see docs/LUA_API.md for typical usage.
ShapePtr MakeWedge(double dx, double dy, double dz, double ltx);

// Hexgonal Prism (e.g. bolt)
/// \brief MakeHexPrism — see docs/LUA_API.md for typical usage.
ShapePtr MakeHexPrism(double acrossFlats, double height);

}  // namespace geometry
