#pragma once
#include <ccad/base/Shape.hpp>

namespace ccad {
namespace geom {

/// \brief Make a wedge
Shape Wedge(double dx, double dy, double dz, double ltx);
}  // namespace geom
}  // namespace ccad
