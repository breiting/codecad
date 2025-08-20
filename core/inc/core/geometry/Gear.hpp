/// \file
/// Geometry API for solid modeling (OCCT wrappers).
#pragma once
#include "geometry/Shape.hpp"

namespace geometry {

// Spur Gear
// z = number of teeth, m = Module (mm), th = Width (Z), bore = hole (mm)
// pressure_deg  (typ. 20°)
/// \brief MakeInvoluteGear — see docs/LUA_API.md for typical usage.
ShapePtr MakeInvoluteGear(int z, double m, double th, double bore, double pressureDeg = 20.0);

}  // namespace geometry
