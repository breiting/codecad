#pragma once
#include "Geo/Shape.hpp"

namespace Geo {

// Rundung an allen Kanten mit konstantem Radius (mm)
ShapePtr FilletAll(const Geo::ShapePtr& s, double radius_mm);

// Fase an allen Kanten mit konstantem Abstand (mm, symmetrisch)
ShapePtr ChamferAll(const Geo::ShapePtr& s, double distance_mm);

}  // namespace Geo
