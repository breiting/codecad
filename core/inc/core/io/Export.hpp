#pragma once
#include <string>

#include "geo/Shape.hpp"

namespace io {

bool SaveSTL(const geo::ShapePtr& shape, const std::string& path, double deflection = 0.1);
bool SaveSTEP(const geo::ShapePtr& shape, const std::string& path);

}  // namespace io
