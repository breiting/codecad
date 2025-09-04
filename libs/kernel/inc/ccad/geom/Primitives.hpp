#pragma once
#include <ccad/geom/Shape.hpp>

namespace ccad {

/** \brief Make an axis-aligned box (origin at (0,0,0)). */
Shape Box(double sx, double sy, double sz);

/** \brief Make a Z-aligned cylinder with diameter d and height h. */
Shape Cylinder(double diameter, double height);

/// \brief Make a sphere with given diameter
Shape Sphere(double diameter);

/// \brief Make a cone
Shape Cone(double diameter1, double diameter2, double height);

/// \brief Make a wedge
Shape Wedge(double dx, double dy, double dz, double ltx);

/// \brief Make a HexPrism (e.g. nut, or bolt head)
Shape HexPrism(double acrossFlats, double height);

}  // namespace ccad
