#pragma once
#include "geo/Shape.hpp"

namespace geo {

// Extrudiert ein Face entlang +Z um 'height' (mm).
// Falls Shape kein Face ist, wird es unverändert zurückgegeben.
ShapePtr ExtrudeZ(const ShapePtr& face, double height);

// Revolve eines Face/Wire um Z-Achse (durch Ursprung) um 'angle_deg' (0..360).
// Für 360° aus Face → Solid.
ShapePtr RevolveZ(const ShapePtr& face_or_wire, double angle_deg);

}  // namespace geo
