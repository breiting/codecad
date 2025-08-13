/// \file
/// Geometry API for solid modeling (OCCT wrappers).
#pragma once
#include "geo/Shape.hpp"

namespace geo {

// Extrudes a face along +Z with the given height (mm)
/// \brief ExtrudeZ — see docs/LUA_API.md for typical usage.
ShapePtr ExtrudeZ(const ShapePtr& face, double height);

// Revolve a face/wire around the Z axis with 'angle_deg' (0..360).
/// \brief RevolveZ — see docs/LUA_API.md for typical usage.
ShapePtr RevolveZ(const ShapePtr& face_or_wire, double angleDeg);

}  // namespace geo
