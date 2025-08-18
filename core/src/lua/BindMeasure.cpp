#include "runtime/BindMeasure.hpp"

#include "geo/Measure.hpp"
#include "geo/Transform.hpp"

namespace runtime {

static sol::table BBoxToTable(sol::state& lua, const geo::BBox& b) {
    sol::table t = lua.create_table();
    t["valid"] = b.isValid;
    sol::table min = lua.create_table();
    min["x"] = b.minX;
    min["y"] = b.minY;
    min["z"] = b.minZ;
    sol::table max = lua.create_table();
    max["x"] = b.maxX;
    max["y"] = b.maxY;
    max["z"] = b.maxZ;
    sol::table size = lua.create_table();
    size["x"] = b.SizeX();
    size["y"] = b.SizeY();
    size["z"] = b.SizeZ();
    sol::table ctr = lua.create_table();
    ctr["x"] = b.Cx();
    ctr["y"] = b.Cy();
    ctr["z"] = b.Cz();
    t["min"] = min;
    t["max"] = max;
    t["size"] = size;
    t["center"] = ctr;
    return t;
}

void RegisterMeasure(sol::state& lua) {
    lua.set_function("bbox", [&lua](const geo::ShapePtr& s, sol::optional<bool> useTri) {
        auto b = geo::ComputeBBox(s, useTri.value_or(true));
        return BBoxToTable(lua, b);
    });

    // Center helpers
    lua.set_function("center_x", [](const geo::ShapePtr& s) {
        auto b = geo::ComputeBBox(s, true);
        if (!b.isValid) return s;
        return geo::Translate(s, -b.Cx(), 0, 0);
    });
    lua.set_function("center_y", [](const geo::ShapePtr& s) {
        auto b = geo::ComputeBBox(s, true);
        if (!b.isValid) return s;
        return geo::Translate(s, 0, -b.Cy(), 0);
    });
    lua.set_function("center_z", [](const geo::ShapePtr& s) {
        auto b = geo::ComputeBBox(s, true);
        if (!b.isValid) return s;
        return geo::Translate(s, 0, 0, -b.Cz());
    });
    lua.set_function("center_xy", [](const geo::ShapePtr& s) {
        auto b = geo::ComputeBBox(s, true);
        if (!b.isValid) return s;
        return geo::Translate(s, -b.Cx(), -b.Cy(), 0);
    });
    lua.set_function("center_xyz", [](const geo::ShapePtr& s) {
        auto b = geo::ComputeBBox(s, true);
        if (!b.isValid) return s;
        return geo::Translate(s, -b.Cx(), -b.Cy(), -b.Cz());
    });
    lua.set_function("center_to", [](const geo::ShapePtr& s, double cx, double cy, double cz) {
        auto b = geo::ComputeBBox(s, true);
        if (!b.isValid) return s;
        return geo::Translate(s, cx - b.Cx(), cy - b.Cy(), cz - b.Cz());
    });
}

}  // namespace runtime
