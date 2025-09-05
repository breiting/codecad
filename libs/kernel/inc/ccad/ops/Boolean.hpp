#pragma once
#include <ccad/base/Shape.hpp>

namespace ccad {
namespace ops {

/** \name Boolean operations
 *  \{ */
Shape Union(const std::vector<Shape>& shapes);
Shape Difference(const Shape& a, const Shape& b);
Shape Intersection(const Shape& a, const Shape& b);
/** \} */

}  // namespace ops
}  // namespace ccad
