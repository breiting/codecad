#include "ccad/lua/BindingUtils.hpp"

#include <stdexcept>

#include "ccad/base/Math.hpp"

namespace ccad {
namespace lua {

std::vector<Vec2> ParsePointTable(sol::table t) {
    std::vector<Vec2> out;
    out.reserve(t.size());
    for (std::size_t i = 1; i <= t.size(); ++i) {
        sol::object row = t[i];
        if (row.get_type() != sol::type::table) {
            throw std::runtime_error("poly: points must be tables like {x, y}");
        }
        sol::table pairtbl = row.as<sol::table>();
        sol::object ox = pairtbl[1];
        sol::object oy = pairtbl[2];
        if (!ox.valid() || !oy.valid()) {
            ox = pairtbl["x"];
            oy = pairtbl["y"];
        }
        if (!ox.valid() || !oy.valid()) {
            throw std::runtime_error("poly: each point must be {x, y} numbers");
        }
        double xv, yv;
        if (ox.is<double>())
            xv = ox.as<double>();
        else if (ox.is<int>())
            xv = static_cast<double>(ox.as<int>());
        else
            throw std::runtime_error("poly: x must be a number");
        if (oy.is<double>())
            yv = oy.as<double>();
        else if (oy.is<int>())
            yv = static_cast<double>(oy.as<int>());
        else
            throw std::runtime_error("poly: y must be a number");
        out.emplace_back(Vec2{xv, yv});
    }
    return out;
}
}  // namespace lua
}  // namespace ccad
