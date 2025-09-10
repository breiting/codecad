#include <ccad/base/Exception.hpp>
#include <ccad/base/Shape.hpp>
#include <ccad/geom/Curves.hpp>
#include <sol/sol.hpp>

#include "ccad/geom/CurvedPlate.hpp"
#include "ccad/lua/BindingUtils.hpp"

using namespace ccad::geom;

namespace ccad {
namespace lua {

void RegisterCurves(sol::state& lua) {
    /**
     * lathe(opts)
     *
     * Lua options table:
     *   points         : { {x,y}, ... }   -- XY silhouette; x=radius, y=height (Z)
     *   angle?         : number in degrees (default 360)
     *   thickness?     : wall thickness in mm; if present -> hollow, else solid
     *
     * Returns: Shape (solid or hollow revolved body)
     */
    lua.set_function("lathe", [](const sol::table& opts) -> Shape {
        // -- read points --
        sol::object oPts = opts.get<sol::object>("points");
        if (!oPts.valid() || !oPts.is<sol::table>()) {
            throw std::runtime_error("lathe: missing 'points' array");
        }
        auto pts = ParsePointTable(oPts.as<sol::table>());

        const double angle = opts.get_or<double, std::string, double>("angle", 360.0);
        const double thickness = opts.get_or<double, std::string, double>("thickness", 0.0);

        // -- build spec --
        LatheSpec spec;
        spec.points = std::move(pts);
        spec.angleDeg = angle;
        spec.thickness = thickness;

        try {
            return Lathe(spec);
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("lathe failed: ") + e.what());
        }
    });

    lua.set_function("curved_plate_xy", [](double sx, double sy, double thickness, double k_u, double k_v) {
        CurvedPlateSpec s{};
        s.sizeX = sx;
        s.sizeY = sy;
        s.thickness = thickness;
        s.nu = 8;
        s.nv = 8;
        s.kU = k_u;
        s.kV = k_v;
        s.dirU = {1.0, 0.0};
        s.dirV = {0.0, 1.0};
        s.law = BendLaw::Paraboloid;
        return CurvedPlate(s);
    });
}

}  // namespace lua
}  // namespace ccad
