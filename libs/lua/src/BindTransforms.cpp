#include <ccad/ops/Transform.hpp>
#include <sol/sol.hpp>

#include "ccad/lua/Bindings.hpp"

using namespace ccad::ops;

namespace ccad {
namespace lua {

void RegisterTransforms(sol::state& lua) {
    lua.set_function("translate",
                     [](const Shape& s, double dx, double dy, double dz) -> Shape { return Translate(s, dx, dy, dz); });
    lua.set_function("rotate_x", [](const Shape& s, double deg) -> Shape { return RotateX(s, deg); });
    lua.set_function("rotate_y", [](const Shape& s, double deg) -> Shape { return RotateY(s, deg); });
    lua.set_function("rotate_z", [](const Shape& s, double deg) -> Shape { return RotateZ(s, deg); });
    lua.set_function("scale", [](const Shape& s, double factor) -> Shape { return ScaleUniform(s, factor); });
}
}  // namespace lua
}  // namespace ccad
