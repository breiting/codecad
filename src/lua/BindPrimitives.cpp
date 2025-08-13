#include "runtime/BindPrimitives.hpp"

#include "geo/Primitives.hpp"

namespace runtime {
void RegisterPrimitives(sol::state& lua) {
    lua.set_function("box", [](double x, double y, double z) -> geo::ShapePtr { return geo::MakeBox(x, y, z); });
    lua.set_function("cylinder", [](double d, double h) -> geo::ShapePtr { return geo::MakeCylinder(d, h); });
    lua.set_function("hex_prism",
                     [](double acrossFlats, double h) -> geo::ShapePtr { return geo::MakeHexPrism(acrossFlats, h); });
}
}  // namespace runtime
