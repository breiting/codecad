#include "runtime/BindFeatures.hpp"

#include <Standard_Failure.hxx>
#include <stdexcept>

#include "geo/Features.hpp"

namespace runtime {
void RegisterFeatures(sol::state& lua) {
    lua.set_function("fillet", [](const geo::ShapePtr& s, double r) -> geo::ShapePtr {
        try {
            return geo::FilletAll(s, r);
        } catch (const Standard_Failure& e) {
            throw std::runtime_error(std::string("fillet failed: ") + e.GetMessageString());
        }
    });
    lua.set_function("chamfer", [](const geo::ShapePtr& s, double d) -> geo::ShapePtr {
        try {
            return geo::ChamferAll(s, d);
        } catch (const Standard_Failure& e) {
            throw std::runtime_error(std::string("chamfer failed: ") + e.GetMessageString());
        }
    });
}
}  // namespace runtime
