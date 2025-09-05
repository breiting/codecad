#pragma once

#include <ccad/base/Shape.hpp>
#include <string>

#include "ccad/geom/Triangulation.hpp"

namespace ccad {
namespace io {

bool SaveSTL(const Shape& shape, const std::string& path, geom::TriangulationParams p);
bool SaveSTEP(const Shape& shape, const std::string& path);

}  // namespace io
}  // namespace ccad
