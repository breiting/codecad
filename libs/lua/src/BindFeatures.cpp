#include <ccad/base/Exception.hpp>
#include <ccad/base/Shape.hpp>
#include <ccad/feature/Chamfer.hpp>
#include <ccad/feature/Fillet.hpp>
#include <sol/sol.hpp>

#include "ccad/lua/Bindings.hpp"

using namespace ccad::feature;

namespace ccad {
namespace lua {

void RegisterFeatures(sol::state& lua) {
    lua.set_function("fillet", [](const Shape& s, double r) -> Shape {
        try {
            return feature::FilletAll(s, r);
        } catch (const Exception& e) {
            throw std::runtime_error(std::string("fillet failed: ") + e.getDescription());
        }
    });
    lua.set_function("chamfer", [](const Shape& s, double d) -> Shape {
        try {
            return feature::ChamferAll(s, d);
        } catch (const Exception& e) {
            throw std::runtime_error(std::string("chamfer failed: ") + e.getDescription());
        }
    });
}
}  // namespace lua
}  // namespace ccad
