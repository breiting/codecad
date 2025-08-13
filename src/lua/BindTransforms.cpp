#include "runtime/BindTransforms.hpp"
#include "geo/Transform.hpp"

namespace runtime {
void RegisterTransforms(sol::state& lua) {
    lua.set_function("translate", [](const geo::ShapePtr& s, double dx, double dy, double dz) -> geo::ShapePtr {
        return geo::Translate(s, dx, dy, dz);
    });
    lua.set_function("rotate_x", [](const geo::ShapePtr& s, double deg) -> geo::ShapePtr {
        return geo::RotateX(s, deg);
    });
    lua.set_function("rotate_y", [](const geo::ShapePtr& s, double deg) -> geo::ShapePtr {
        return geo::RotateY(s, deg);
    });
    lua.set_function("rotate_z", [](const geo::ShapePtr& s, double deg) -> geo::ShapePtr {
        return geo::RotateZ(s, deg);
    });
    lua.set_function("scale", [](const geo::ShapePtr& s, double factor) -> geo::ShapePtr {
        return geo::ScaleUniform(s, factor);
    });
}
} // namespace runtime
