#pragma once
#include <ccad/base/Shape.hpp>

namespace ccad {
namespace sketch {

/** \brief Make a 2D rectangle with the given size, centered around origin in the XY plane. */
Shape Rectangle(double width, double height);

}  // namespace sketch
}  // namespace ccad
