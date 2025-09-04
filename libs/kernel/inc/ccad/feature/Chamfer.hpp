#pragma once
#include <ccad/base/Shape.hpp>

namespace ccad {
namespace feature {

//
/// \brief Apply a chamfer on all edges with the given distance
Shape ChamferAll(const Shape& s, double distanceMm);
}  // namespace feature
}  // namespace ccad
