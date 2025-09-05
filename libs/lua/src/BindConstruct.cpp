#include <ccad/base/Shape.hpp>
#include <ccad/construct/Extrude.hpp>
#include <ccad/construct/Revolve.hpp>
#include <sol/sol.hpp>

#include "ccad/lua/Bindings.hpp"

using namespace ccad::construct;

namespace ccad {
namespace lua {

void RegisterConstruct(sol::state& lua) {
    lua.set_function("extrude",
                     [](const Shape& face, double height) -> Shape { return construct::ExtrudeZ(face, height); });
    lua.set_function("revolve",
                     [](const Shape& prof, double angleDeg) -> Shape { return construct::RevolveZ(prof, angleDeg); });
}
}  // namespace lua
}  // namespace ccad
