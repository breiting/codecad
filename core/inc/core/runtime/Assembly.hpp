#pragma once
#include <string>
#include <vector>

#include "geo/Shape.hpp"

namespace runtime {

struct Part {
    std::string name;
    geo::ShapePtr shape;
    double ex = 0.0, ey = 0.0, ez = 0.0;  // explode vector
};

struct Assembly {
    std::string name;
    std::vector<Part> parts;
};

}  // namespace runtime
