#pragma once
#include <ccad/base/Shape.hpp>

namespace ccad {
namespace geom {

/// \brief Make a HexPrism (e.g. nut, or bolt head)
Shape HexPrism(double acrossFlats, double height);
}  // namespace geom
}  // namespace ccad
