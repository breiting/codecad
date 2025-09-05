#pragma once
#include <ccad/base/Shape.hpp>

namespace ccad {
namespace feature {

enum class ChamferRadialType { Undefined, External, Internal };

struct ChamferRadialSpec {
    ChamferRadialType type = ChamferRadialType::Undefined;
    double angleDeg = 45;  ///< Angle from XY plane towards Z
    double length = 2;     ///< Length of the chamfer (e.g. 2mm)
};

//
/// \brief Apply a chamfer on all edges with the given distance
Shape ChamferAll(const Shape& s, double distanceMm);

/// \brief Generates a radial chamfer cutter for rods (external) or holes (internal)
Shape ChamferCutterRadial(double diameter, const ChamferRadialSpec& spec);

}  // namespace feature
}  // namespace ccad
