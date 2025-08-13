#include "runtime/BindBooleans.hpp"

#include "geo/Boolean.hpp"
using geo::ShapePtr;

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
        return geo::MakeUnion(shapes);
    });

    lua.set_function("difference", [](const geo::ShapePtr& a, const geo::ShapePtr& b) -> geo::ShapePtr {
        return geo::MakeDifference(a, b);
    });

    lua.set_function("intersection", [](const geo::ShapePtr& a, const geo::ShapePtr& b) -> geo::ShapePtr {
        return geo::MakeIntersect(a, b);
    });
}

}  // namespace runtime
