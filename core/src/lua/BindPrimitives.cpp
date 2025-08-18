#include "runtime/BindPrimitives.hpp"

#include "geometry/Primitives.hpp"

namespace runtime {
void RegisterPrimitives(sol::state& lua) {
    lua.set_function("box", [](double x, double y, double z) -> geometry::ShapePtr { return geometry::MakeBox(x, y, z); });
    lua.set_function("cylinder", [](double d, double h) -> geometry::ShapePtr { return geometry::MakeCylinder(d, h); });
    lua.set_function("cone", [](double d1, double d2, double h) -> geometry::ShapePtr { return geometry::MakeCone(d1, d2, h); });
    lua.set_function("wedge", [](double dx, double dy, double dz, double ltx) -> geometry::ShapePtr {
        return geometry::MakeWedge(dx, dy, dz, ltx);
    });
    lua.set_function("sphere", [](double d) -> geometry::ShapePtr { return geometry::MakeSphere(d); });
    lua.set_function("hex_prism",
                     [](double acrossFlats, double h) -> geometry::ShapePtr { return geometry::MakeHexPrism(acrossFlats, h); });
}
}  // namespace runtime
