#include "runtime/BindConstruct.hpp"

#include "geometry/Extrude.hpp"

namespace runtime {
void RegisterConstruct(sol::state& lua) {
    lua.set_function("extrude", [](const geometry::ShapePtr& face, double height) -> geometry::ShapePtr {
        return geometry::ExtrudeZ(face, height);
    });
    lua.set_function("revolve", [](const geometry::ShapePtr& prof, double angleDeg) -> geometry::ShapePtr {
        return geometry::RevolveZ(prof, angleDeg);
    });
}
}  // namespace runtime
