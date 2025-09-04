#pragma once
#include <ccad/base/Shape.hpp>

namespace ccad {
namespace feature {

//
/// \brief Round-off edges with constant radius (mm)
Shape FilletAll(const Shape& s, double radiusMm);

}  // namespace feature
}  // namespace ccad
