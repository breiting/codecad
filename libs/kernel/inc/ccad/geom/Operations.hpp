#pragma once
#include <ccad/geom/Shape.hpp>

namespace ccad {
namespace op {

/** \name Boolean operations
 *  \{ */
Shape Union(const Shape& a, const Shape& b);
Shape Difference(const Shape& a, const Shape& b);
Shape Intersection(const Shape& a, const Shape& b);
/** \} */

/** \name Transforms (returns new shapes)
 *  \{ */
Shape Translate(const Shape& s, double dx, double dy, double dz);
Shape RotateX(const Shape& s, double deg);
Shape RotateY(const Shape& s, double deg);
Shape RotateZ(const Shape& s, double deg);
Shape ScaleUniform(const Shape& s, double factor);
/** \} */

}  // namespace op
}  // namespace ccad
