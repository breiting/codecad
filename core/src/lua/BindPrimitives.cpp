#include "runtime/BindPrimitives.hpp"

#include "geo/Primitives.hpp"

namespace runtime {
void RegisterPrimitives(sol::state& lua) {
    lua.set_function("box", [](double x, double y, double z) -> geo::ShapePtr { return geo::MakeBox(x, y, z); });
    lua.set_function("cylinder", [](double d, double h) -> geo::ShapePtr { return geo::MakeCylinder(d, h); });
    lua.set_function("cone", [](double d1, double d2, double h) -> geo::ShapePtr { return geo::MakeCone(d1, d2, h); });
    lua.set_function("wedge", [](double dx, double dy, double dz, double ltx) -> geo::ShapePtr {
        return geo::MakeWedge(dx, dy, dz, ltx);
    });
    lua.set_function("sphere", [](double d) -> geo::ShapePtr { return geo::MakeSphere(d); });
    lua.set_function("hex_prism",
                     [](double acrossFlats, double h) -> geo::ShapePtr { return geo::MakeHexPrism(acrossFlats, h); });
}
}  // namespace runtime
