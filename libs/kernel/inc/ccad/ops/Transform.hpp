#pragma once
#include <ccad/base/Shape.hpp>

namespace ccad {
namespace ops {

/** \name Transforms (returns new shapes)
 *  \{ */
Shape Translate(const Shape& s, double dx, double dy, double dz);
Shape RotateX(const Shape& s, double deg);
Shape RotateY(const Shape& s, double deg);
Shape RotateZ(const Shape& s, double deg);
Shape ScaleUniform(const Shape& s, double factor);
/** \} */

}  // namespace ops
}  // namespace ccad
