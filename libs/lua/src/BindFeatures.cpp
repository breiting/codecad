#include "runtime/BindFeatures.hpp"

#include <Standard_Failure.hxx>
#include <stdexcept>

#include "geometry/Features.hpp"

namespace runtime {
void RegisterFeatures(sol::state& lua) {
    lua.set_function("fillet", [](const geometry::ShapePtr& s, double r) -> geometry::ShapePtr {
        try {
            return geometry::FilletAll(s, r);
        } catch (const Standard_Failure& e) {
            throw std::runtime_error(std::string("fillet failed: ") + e.GetMessageString());
        }
    });
    lua.set_function("chamfer", [](const geometry::ShapePtr& s, double d) -> geometry::ShapePtr {
        try {
            return geometry::ChamferAll(s, d);
        } catch (const Standard_Failure& e) {
            throw std::runtime_error(std::string("chamfer failed: ") + e.GetMessageString());
        }
    });
}
}  // namespace runtime
