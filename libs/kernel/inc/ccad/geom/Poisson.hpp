#pragma once
#include <ccad/base/Shape.hpp>

#include "ccad/base/PoissonDisk.hpp"

namespace ccad {
namespace geom {

/** \brief Make a plate with holes based on Poisson Disk sampling. */
Shape Poisson(const PoissonDiskSpec& spec, double thickness, const PoissonResult& points);

}  // namespace geom
}  // namespace ccad
