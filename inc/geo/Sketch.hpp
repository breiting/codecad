/// \file
/// Geometry API for solid modeling (OCCT wrappers).
#pragma once
#include <utility>
#include <vector>

#include "geo/Shape.hpp"

namespace geo {

// Polyline (filled face) on XY plane: points(x,y), z=0. If closed=true, the polyline will get closed automatically
/// \brief PolylineXY_Face — see docs/LUA_API.md for typical usage.
ShapePtr PolylineXY_Face(const std::vector<std::pair<double, double>>& pts, bool closed);

// Polyline (filled face) on XZ profile space: points(r,z) → 3D (x=r, y=0, z=z).
/// \brief PolylineXZ_Face — see docs/LUA_API.md for typical usage.
ShapePtr PolylineXZ_Face(const std::vector<std::pair<double, double>>& rz, bool closed, bool closeToAxis);

}  // namespace geo
