#include "runtime/BindConstruct.hpp"

#include "geo/Extrude.hpp"

namespace runtime {
void RegisterConstruct(sol::state& lua) {
    lua.set_function("extrude", [](const geo::ShapePtr& face, double height) -> geo::ShapePtr {
        return geo::ExtrudeZ(face, height);
    });
    lua.set_function("revolve", [](const geo::ShapePtr& prof, double angleDeg) -> geo::ShapePtr {
        return geo::RevolveZ(prof, angleDeg);
    });
}
}  // namespace runtime
