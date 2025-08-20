#include "runtime/BindTransforms.hpp"
#include "geometry/Transform.hpp"

namespace runtime {
void RegisterTransforms(sol::state& lua) {
    lua.set_function("translate", [](const geometry::ShapePtr& s, double dx, double dy, double dz) -> geometry::ShapePtr {
        return geometry::Translate(s, dx, dy, dz);
    });
    lua.set_function("rotate_x", [](const geometry::ShapePtr& s, double deg) -> geometry::ShapePtr {
        return geometry::RotateX(s, deg);
    });
    lua.set_function("rotate_y", [](const geometry::ShapePtr& s, double deg) -> geometry::ShapePtr {
        return geometry::RotateY(s, deg);
    });
    lua.set_function("rotate_z", [](const geometry::ShapePtr& s, double deg) -> geometry::ShapePtr {
        return geometry::RotateZ(s, deg);
    });
    lua.set_function("scale", [](const geometry::ShapePtr& s, double factor) -> geometry::ShapePtr {
        return geometry::ScaleUniform(s, factor);
    });
}
} // namespace runtime
