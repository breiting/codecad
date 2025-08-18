#include "runtime/BindDraft.hpp"

#include <string>

#include "geometry/Draft.hpp"

namespace runtime {

static sol::object PolysToLua(sol::state& lua, const std::vector<geometry::Polyline2D>& polys) {
    sol::table arr = lua.create_table();
    int i = 1;
    for (auto& pl : polys) {
        sol::table t = lua.create_table();
        int j = 1;
        for (auto& pt : pl) {
            sol::table p = lua.create_table();
            p[1] = pt.first;   // u
            p[2] = pt.second;  // v
            t[j++] = p;
        }
        arr[i++] = t;
    }
    return sol::make_object(lua, arr);
}

void RegisterDraft(sol::state& lua) {
    // Overload 1: section_outline(shape, axis:string<'x'|'y'|'z'>, value:number [, defl:number])
    lua.set_function("section_outline",
                     [&lua](const geometry::ShapePtr& s, const std::string& axis, double value, sol::optional<double> defl) {
                         char a = axis.empty() ? 'z' : static_cast<char>(axis[0]);
                         auto polys = geometry::SectionOutline2D(s, a, value, defl.value_or(0.2));
                         return PolysToLua(lua, polys);
                     });
}

}  // namespace runtime
