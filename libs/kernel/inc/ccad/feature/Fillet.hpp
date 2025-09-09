#pragma once
#include <ccad/base/Shape.hpp>
#include <ccad/select/EdgeSelector.hpp>

namespace ccad {
namespace feature {

//
/// \brief Round-off edges with constant radius (mm)
Shape FilletAll(const Shape& s, double radiusMm);

//
/// Fillet with constant radius on the given edges of a shape.
/// Works for 3D (solids / shells). If the input is a Wire or Face, falls back to 2D fillet.
Shape Fillet(const Shape& s, const select::EdgeSet& edges, double radiusMm);

}  // namespace feature
}  // namespace ccad
