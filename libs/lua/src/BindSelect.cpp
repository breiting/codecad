#include <ccad/base/Exception.hpp>
#include <ccad/base/Shape.hpp>
#include <ccad/select/EdgeSelector.hpp>
#include <optional>
#include <sol/sol.hpp>

using namespace ccad;
using namespace ccad::select;

namespace {

struct LuaEdgeQuery {
    std::optional<EdgeSelector> sel;

    EdgeSelector& S() {
        if (!sel) throw std::runtime_error("edges(): call :from(shape) first");
        return *sel;
    }

    LuaEdgeQuery& from(const Shape& s) {
        sel.emplace(EdgeSelector::FromShape(s));
        return *this;
    }

    // "zmin", "zmax", "xmin", "xmax", "ymin", "ymax"
    LuaEdgeQuery& on_box_side(const std::string& side) {
        std::string k = side;
        for (auto& c : k) c = (char)std::tolower(c);
        BoxSide bs = BoxSide::ZMin;
        if (k == "zmin")
            bs = BoxSide::ZMin;
        else if (k == "zmax")
            bs = BoxSide::ZMax;
        else if (k == "xmin")
            bs = BoxSide::XMin;
        else if (k == "xmax")
            bs = BoxSide::XMax;
        else if (k == "ymin")
            bs = BoxSide::YMin;
        else if (k == "ymax")
            bs = BoxSide::YMax;
        S().onBoxSide(bs);
        return *this;
    }

    // "line" | "circle"
    LuaEdgeQuery& geom(const std::string& kind) {
        std::string k = kind;
        for (auto& c : k) c = (char)std::tolower(c);
        if (k == "line")
            S().geom(EdgeGeom::Line);
        else if (k == "circle")
            S().geom(EdgeGeom::Circle);
        return *this;
    }

    // axis: "x" | "y" | "z", optional tol in degrees (default 3°)
    LuaEdgeQuery& parallel(const std::string& axis, sol::optional<double> tol_deg) {
        double tol = tol_deg.value_or(3.0);
        Axis a = Axis::X;
        if (axis == "x" || axis == "X")
            a = Axis::X;
        else if (axis == "y" || axis == "Y")
            a = Axis::Y;
        else if (axis == "z" || axis == "Z")
            a = Axis::Z;
        S().parallelTo(a, tol);
        return *this;
    }

    // dihedral angle within [min,max] degrees
    LuaEdgeQuery& dihedral_between(double min_deg, double max_deg) {
        S().dihedralBetween(AngleRange({min_deg, max_deg}));
        return *this;
    }

    // length between [min,max] (mm)
    LuaEdgeQuery& length_between(double min_mm, double max_mm) {
        S().lengthBetween(LengthRange({min_mm, max_mm}));
        return *this;
    }

    EdgeSet collect() {
        return S().collect();
    }
};

}  // namespace

namespace ccad {
namespace lua {

void RegisterSelect(sol::state& lua) {
    lua.new_usertype<EdgeSet>("EdgeSet", sol::no_constructor);

    lua.new_usertype<LuaEdgeQuery>("EdgeQuery", sol::constructors<LuaEdgeQuery()>(), "from", &LuaEdgeQuery::from,
                                   "on_box_side", &LuaEdgeQuery::on_box_side, "geom", &LuaEdgeQuery::geom, "parallel",
                                   &LuaEdgeQuery::parallel, "dihedral_between", &LuaEdgeQuery::dihedral_between,
                                   "length_between", &LuaEdgeQuery::length_between, "collect", &LuaEdgeQuery::collect);

    lua.set_function("edges", sol::overload([]() { return LuaEdgeQuery{}; },
                                            [](const Shape& s) {
                                                LuaEdgeQuery q;
                                                q.from(s);
                                                return q;
                                            }));
}
}  // namespace lua
}  // namespace ccad
