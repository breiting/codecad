#pragma once
#include "geo/Shape.hpp"

namespace geo {

// Stirnrad (Spur Gear) mit Involutflanke, Vollkörper durch Extrude.
// z = Zähnezahl, m = Modul (mm), th = Breite (Z), bore = Bohrung (mm)
// pressure_deg = Eingriffswinkel (typ. 20°)
ShapePtr MakeInvoluteGear(int z, double m, double th, double bore, double pressure_deg = 20.0);

}  // namespace geo
