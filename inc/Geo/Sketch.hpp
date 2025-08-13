#pragma once
#include <utility>
#include <vector>

#include "geo/Shape.hpp"

namespace geo {

// Polyline auf XY-Ebene: Punkte (x,y), z=0. Wenn closed=true, wird automatisch geschlossen.
// Erzeugt eine Face (gefüllt). Für reine Wire später separat, falls nötig.
ShapePtr PolylineXY_Face(const std::vector<std::pair<double, double>>& pts, bool closed);

// Polyline im XZ-„Profilraum“: Punkte (r,z) → 3D (x=r, y=0, z=z).
// Für Revolve: Wenn close_to_axis=true, wird ein Rand zum Z-Axis (r=0) ergänzt,
// damit Revolve(Face, 360°) einen Solid erzeugt.
ShapePtr PolylineXZ_Face(const std::vector<std::pair<double, double>>& rz, bool closed, bool close_to_axis);

}  // namespace geo
