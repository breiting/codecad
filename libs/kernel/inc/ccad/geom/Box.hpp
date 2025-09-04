#pragma once
#include <ccad/base/Shape.hpp>

namespace ccad {
namespace geom {

/** \brief Make an axis-aligned box (origin at (0,0,0)). */
Shape Box(double sx, double sy, double sz);

}  // namespace geom
}  // namespace ccad
