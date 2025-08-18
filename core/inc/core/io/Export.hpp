#pragma once
#include <string>

#include "geometry/Shape.hpp"

namespace io {

bool SaveSTL(const geometry::ShapePtr& shape, const std::string& path, double deflection = 0.1);
bool SaveSTEP(const geometry::ShapePtr& shape, const std::string& path);

}  // namespace io
