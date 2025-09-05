#include <ccad/base/Math.hpp>
#include <ccad/base/Shape.hpp>
#include <ccad/ops/Transform.hpp>
#include <sol/sol.hpp>

#include "ccad/lua/Bindings.hpp"

using namespace ccad;
using namespace ccad::ops;

namespace ccad {
namespace lua {

static sol::table BBoxToTable(sol::state& lua, const Bounds& b) {
    sol::table t = lua.create_table();
    t["valid"] = b.IsValid();
    sol::table min = lua.create_table();
    min["x"] = b.min.x;
    min["y"] = b.min.y;
    min["z"] = b.min.z;
    sol::table max = lua.create_table();
    max["x"] = b.max.x;
    max["y"] = b.max.y;
    max["z"] = b.max.z;
    sol::table size = lua.create_table();
    size["x"] = b.Size().x;
    size["y"] = b.Size().y;
    size["z"] = b.Size().z;
    sol::table ctr = lua.create_table();
    ctr["x"] = b.Center().x;
    ctr["y"] = b.Center().y;
    ctr["z"] = b.Center().z;
    t["min"] = min;
    t["max"] = max;
    t["size"] = size;
    t["center"] = ctr;
    return t;
}

void RegisterMeasure(sol::state& lua) {
    lua.set_function("bbox", [&lua](const Shape& s) {
        auto b = s.BBox();
        return BBoxToTable(lua, b);
    });

    // Center helpers
    lua.set_function("center_x", [](const Shape& s) {
        auto b = s.BBox();
        if (!b.IsValid()) return s;
        return Translate(s, -b.Center().x, 0, 0);
    });
    lua.set_function("center_y", [](const Shape& s) {
        auto b = s.BBox();
        if (!b.IsValid()) return s;
        return Translate(s, 0, -b.Center().y, 0);
    });
    lua.set_function("center_z", [](const Shape& s) {
        auto b = s.BBox();
        if (!b.IsValid()) return s;
        return Translate(s, 0, 0, -b.Center().z);
    });
    lua.set_function("center_xy", [](const Shape& s) {
        auto b = s.BBox();
        if (!b.IsValid()) return s;
        return Translate(s, -b.Center().x, -b.Center().y, 0);
    });
    lua.set_function("center_xyz", [](const Shape& s) {
        auto b = s.BBox();
        if (!b.IsValid()) return s;
        return Translate(s, -b.Center().x, -b.Center().y, -b.Center().z);
    });
    lua.set_function("center_to", [](const Shape& s, double cx, double cy, double cz) {
        auto b = s.BBox();
        if (!b.IsValid()) return s;
        return Translate(s, cx - b.Center().x, cy - b.Center().y, cz - b.Center().z);
    });
}
}  // namespace lua
}  // namespace ccad
