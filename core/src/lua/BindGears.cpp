#include "runtime/BindGears.hpp"

#include <Standard_Failure.hxx>
#include <stdexcept>

#include "geometry/Gear.hpp"

namespace runtime {
void RegisterGears(sol::state& lua) {
    lua.set_function("gear_involute",
                     [](int z, double m, double th, double bore, sol::object pressureDeg) -> geometry::ShapePtr {
                         try {
                             double p = pressureDeg.valid() ? pressureDeg.as<double>() : 20.0;
                             return geometry::MakeInvoluteGear(z, m, th, bore, p);
                         } catch (const Standard_Failure& e) {
                             throw std::runtime_error(std::string("gear_involute failed: ") + e.GetMessageString());
                         }
                     });
}
}  // namespace runtime
