#include "runtime/BindSketch.hpp"

#include "geometry/Sketch.hpp"
#include "runtime/BindingUtils.hpp"

namespace runtime {
void RegisterSketch(sol::state& lua) {
    lua.set_function("poly_xy", [](sol::table pts_tbl, sol::optional<bool> closedOpt) -> geometry::ShapePtr {
        auto pts = ParsePointTable(pts_tbl);
        bool closed = closedOpt.value_or(false);
        return geometry::PolylineXY_Face(pts, closed);
    });

    lua.set_function(
        "poly_xz",
        [](sol::table rz_tbl, sol::optional<bool> closedOpt, sol::optional<bool> closeToAxisOpt) -> geometry::ShapePtr {
            auto rz = ParsePointTable(rz_tbl);
            bool closed = closedOpt.value_or(false);
            bool close_to_axis = closeToAxisOpt.value_or(false);
            return geometry::PolylineXZ_Face(rz, closed, close_to_axis);
        });
}
}  // namespace runtime
