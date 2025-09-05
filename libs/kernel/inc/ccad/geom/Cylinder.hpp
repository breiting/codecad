#pragma once
#include <ccad/base/Shape.hpp>

namespace ccad {
namespace geom {

/** \brief Make a Z-aligned cylinder with diameter d and height h. */
Shape Cylinder(double diameter, double height);
}  // namespace geom
}  // namespace ccad
