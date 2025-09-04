#pragma once
#include <ccad/base/Shape.hpp>

namespace ccad {
namespace construct {

//
/// \brief Extrudes a face along +Z with the given height (mm)
Shape ExtrudeZ(const Shape& face, double height);

}  // namespace construct
}  // namespace ccad
