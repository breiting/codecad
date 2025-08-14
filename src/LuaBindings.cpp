#include "runtime/LuaBindings.hpp"

#include "runtime/BindBooleans.hpp"
#include "runtime/BindConstruct.hpp"
#include "runtime/BindFeatures.hpp"
#include "runtime/BindGears.hpp"
#include "runtime/BindIO.hpp"
#include "runtime/BindMeasure.hpp"
#include "runtime/BindPrimitives.hpp"
#include "runtime/BindSketch.hpp"
#include "runtime/BindTransforms.hpp"

namespace runtime {

LuaBindings::LuaBindings() : m_Emitted(nullptr) {
}

void LuaBindings::Register(sol::state& lua) {
    // Core libs
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string, sol::lib::package);

    // package.path augmentation for ./lib
    std::string extra = "./lib/?.lua;./lib/?/init.lua;";
    std::string cur = lua["package"]["path"];
    lua["package"]["path"] = extra + cur;

    // PARAMS table + helpers
    sol::table params = lua.create_named_table("PARAMS");
    lua.set_function("mm", [](double v) { return v; });
    lua.set_function("deg", [](double v) { return v; });
    lua.set_function("param", [&lua](const std::string& name, sol::object def) -> sol::object {
        sol::table p = lua["PARAMS"];
        sol::object v = p[name];
        if (v.valid() && v.get_type() != sol::type::nil && v.get_type() != sol::type::none) {
            // number
            if (def.is<double>() && v.is<std::string>()) {
                try {
                    return sol::make_object(lua, std::stod(v.as<std::string>()));
                } catch (...) {
                }
            }
            // boolean
            if (def.is<bool>() && v.is<std::string>()) {
                std::string s = v.as<std::string>();
                for (auto& c : s) c = (char)std::tolower(c);
                bool b = (s == "1" || s == "true" || s == "yes" || s == "on");
                return sol::make_object(lua, b);
            }
            return v;
        }
        return def;
    });

    // Register modules
    RegisterPrimitives(lua);
    RegisterTransforms(lua);
    RegisterBooleans(lua);
    RegisterFeatures(lua);
    RegisterSketch(lua);
    RegisterConstruct(lua);
    RegisterGears(lua);
    RegisterMeasure(lua);
    RegisterIO(lua, this);
}

}  // namespace runtime
