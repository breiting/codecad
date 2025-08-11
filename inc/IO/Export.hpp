#pragma once
#include <string>

#include "Geo/Shape.hpp"

namespace IO {

bool SaveSTL(const Geo::ShapePtr& shape, const std::string& path, double deflection = 0.1);
bool SaveSTEP(const Geo::ShapePtr& shape, const std::string& path);

}  // namespace IO
