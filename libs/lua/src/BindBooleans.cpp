#include <ccad/ops/Boolean.hpp>
#include <sol/sol.hpp>

#include "ccad/lua/Bindings.hpp"

using namespace ccad::ops;

namespace ccad {
namespace lua {

void RegisterBooleans(sol::state& lua) {
    lua.set_function("union", [](sol::variadic_args va) -> Shape {
        std::vector<Shape> shapes;
        shapes.reserve(va.size());
        for (auto v : va) {
            if (v.is<Shape>()) {
                shapes.push_back(v.as<Shape>());
            } else {
                throw std::runtime_error("union(...) expects shapes");
            }
        }
        if (shapes.size() < 2) {
            throw std::runtime_error("union(...) expects at least 2 shapes");
        }
        return Union(shapes);
    });

    lua.set_function("difference", [](const Shape& a, const Shape& b) -> Shape { return Difference(a, b); });

    lua.set_function("intersection", [](const Shape& a, const Shape& b) -> Shape { return Intersection(a, b); });
}
}  // namespace lua
}  // namespace ccad
