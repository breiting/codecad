#include <ccad/base/Shape.hpp>
#include <ccad/sketch/Rectangle.hpp>
#include <ccad/sketch/SketchProfiles.hpp>
#include <sol/sol.hpp>

#include "ccad/lua/BindingUtils.hpp"
#include "ccad/lua/Bindings.hpp"

using namespace ccad::sketch;

namespace ccad {
namespace lua {

void RegisterSketch(sol::state& lua) {
    lua.set_function("rect", [](double width, double height) -> Shape { return sketch::Rectangle(width, height); });

    lua.set_function("poly_xy", [](sol::table pts_tbl) -> Shape {
        auto pts = ParsePointTable(pts_tbl);
        return PolyXY(pts);
    });

    lua.set_function("profile_xz", [](sol::table rz_tbl, sol::optional<bool> closedOpt) -> Shape {
        auto rz = ParsePointTable(rz_tbl);
        bool closed = closedOpt.value_or(false);
        return ProfileXZ(rz, closed);
    });
}
}  // namespace lua
}  // namespace ccad
