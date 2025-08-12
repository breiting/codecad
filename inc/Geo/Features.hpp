#pragma once
#include "geo/Shape.hpp"

namespace geo {

// Rundung an allen Kanten mit konstantem Radius (mm)
ShapePtr FilletAll(const ShapePtr& s, double radius_mm);

// Fase an allen Kanten mit konstantem Abstand (mm, symmetrisch)
ShapePtr ChamferAll(const ShapePtr& s, double distance_mm);

}  // namespace geo
