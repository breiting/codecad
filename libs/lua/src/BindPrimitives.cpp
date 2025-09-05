#include <ccad/base/Shape.hpp>
#include <ccad/geom/Box.hpp>
#include <ccad/geom/Cone.hpp>
#include <ccad/geom/Cylinder.hpp>
#include <ccad/geom/HexPrism.hpp>
#include <ccad/geom/Sphere.hpp>
#include <ccad/geom/Wedge.hpp>
#include <sol/sol.hpp>

#include "ccad/lua/Bindings.hpp"

using namespace ccad::geom;

namespace ccad {
namespace lua {

void RegisterPrimitives(sol::state& lua) {
    lua.set_function("box", [](double x, double y, double z) -> Shape { return Box(x, y, z); });

    lua.set_function("cylinder", [](double d, double h) -> Shape { return Cylinder(d, h); });
    lua.set_function("cone", [](double d1, double d2, double h) -> Shape { return Cone(d1, d2, h); });
    lua.set_function("wedge",
                     [](double dx, double dy, double dz, double ltx) -> Shape { return Wedge(dx, dy, dz, ltx); });
    lua.set_function("sphere", [](double d) -> Shape { return Sphere(d); });
    lua.set_function("hex_prism", [](double acrossFlats, double h) -> Shape { return HexPrism(acrossFlats, h); });
}
}  // namespace lua
}  // namespace ccad
