#pragma once
#include "Geo/Shape.hpp"

namespace Geo {

ShapePtr Translate(const ShapePtr& s, double dx, double dy, double dz);
ShapePtr RotateX(const ShapePtr& s, double degrees);
ShapePtr RotateY(const ShapePtr& s, double degrees);
ShapePtr RotateZ(const ShapePtr& s, double degrees);
ShapePtr Scale(const ShapePtr& s, double factor);

}  // namespace Geo
