#pragma once
#include "ccad/geom/Shape.hpp"

namespace ccad {

/** \brief Make an axis-aligned box (origin at (0,0,0)). */
Shape Box(double sx, double sy, double sz);

/** \brief Make a Z-aligned cylinder with diameter d and height h. */
Shape Cylinder(double diameter, double height);

}  // namespace ccad
