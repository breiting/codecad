/// \file
/// Geometry API for solid modeling (OCCT wrappers).
#pragma once
#include "geo/Shape.hpp"

namespace geo {

/// \brief Translate — see docs/LUA_API.md for typical usage.
ShapePtr Translate(const ShapePtr& s, double dx, double dy, double dz);
/// \brief RotateX — see docs/LUA_API.md for typical usage.
ShapePtr RotateX(const ShapePtr& s, double degrees);
/// \brief RotateY — see docs/LUA_API.md for typical usage.
ShapePtr RotateY(const ShapePtr& s, double degrees);
/// \brief RotateZ — see docs/LUA_API.md for typical usage.
ShapePtr RotateZ(const ShapePtr& s, double degrees);
/// \brief Scale — see docs/LUA_API.md for typical usage.
ShapePtr Scale(const ShapePtr& s, double factor);

}  // namespace geo