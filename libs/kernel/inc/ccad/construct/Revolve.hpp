#pragma once
#include <ccad/base/Shape.hpp>

namespace ccad {
namespace construct {

//
/// \brief Revolve a face/wire around the Z axis with 'angle_deg' (0..360).
Shape RevolveZ(const Shape& faceOrWire, double angleDeg);
}  // namespace construct
}  // namespace ccad
