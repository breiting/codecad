#include "runtime/BindBooleans.hpp"

#include "geometry/Boolean.hpp"
using geometry::ShapePtr;

namespace runtime {

void RegisterBooleans(sol::state& lua) {
    lua.set_function("union", [](sol::variadic_args va) -> ShapePtr {
        std::vector<ShapePtr> shapes;
        shapes.reserve(va.size());
        for (auto v : va) {
            if (v.is<ShapePtr>()) {
                shapes.push_back(v.as<ShapePtr>());
            } else {
                throw std::runtime_error("union(...) expects shapes");
            }
        }
        if (shapes.size() < 2) {
            throw std::runtime_error("union(...) expects at least 2 shapes");
        }
        return geometry::MakeUnion(shapes);
    });

    lua.set_function("difference", [](const geometry::ShapePtr& a, const geometry::ShapePtr& b) -> geometry::ShapePtr {
        return geometry::MakeDifference(a, b);
    });

    lua.set_function("intersection", [](const geometry::ShapePtr& a, const geometry::ShapePtr& b) -> geometry::ShapePtr {
        return geometry::MakeIntersect(a, b);
    });
}

}  // namespace runtime
