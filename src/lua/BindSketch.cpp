#include "runtime/BindSketch.hpp"

#include "geo/Sketch.hpp"
#include "runtime/BindingUtils.hpp"

namespace runtime {
void RegisterSketch(sol::state& lua) {
    lua.set_function("poly_xy", [](sol::table pts_tbl, sol::optional<bool> closedOpt) -> geo::ShapePtr {
        auto pts = ParsePointTable(pts_tbl);
        bool closed = closedOpt.value_or(false);
        return geo::PolylineXY_Face(pts, closed);
    });

    lua.set_function(
        "poly_xz",
        [](sol::table rz_tbl, sol::optional<bool> closedOpt, sol::optional<bool> closeToAxisOpt) -> geo::ShapePtr {
            auto rz = ParsePointTable(rz_tbl);
            bool closed = closedOpt.value_or(false);
            bool close_to_axis = closeToAxisOpt.value_or(false);
            return geo::PolylineXZ_Face(rz, closed, close_to_axis);
        });
}
}  // namespace runtime
