#include "runtime/BindAssembly.hpp"

#include "runtime/Assembly.hpp"

namespace runtime {

void RegisterAssembly(sol::state& lua) {
    auto asm_t = lua.new_usertype<Assembly>("Assembly");

    asm_t.set_function("get_name", [](Assembly& a) { return a.name; });
    asm_t.set_function("set_name", [](Assembly& a, const std::string& n) { a.name = n; });
    asm_t.set_function("count", [](Assembly& a) { return (int)a.parts.size(); });
    asm_t.set_function("get_part", [](Assembly& a, int i) {
        if (i < 1 || (size_t)i > a.parts.size()) return Part{};
        return a.parts[(size_t)i - 1];
    });
    asm_t.set_function("clear", [](Assembly& a) { a.parts.clear(); });

    auto part_t = lua.new_usertype<Part>("Part");

    part_t.set_function("set_name", [](Part& p, const std::string& n) { p.name = n; });
    part_t.set_function("set_shape", [](Part& p, const geometry::ShapePtr& s) { p.shape = s; });
    part_t.set_function("set_explosion_vector", [](Part& p, double x, double y, double z) {
        p.ex = x;
        p.ey = y;
        p.ez = z;
    });

    lua.set_function("assembly", [](const std::string& n) {
        Assembly a;
        a.name = n;
        return a;
    });
    lua.set_function(
        "part", [](const std::string& name, const geometry::ShapePtr& s, sol::object ex, sol::object ey, sol::object ez) {
            Part p;
            p.name = name;
            p.shape = s;
            if (ex.valid()) p.ex = ex.as<double>();
            if (ey.valid()) p.ey = ey.as<double>();
            if (ez.valid()) p.ez = ez.as<double>();
            return p;
        });
    lua.set_function("add_part", [](Assembly& a, const Part& p) { a.parts.push_back(p); });
}

}  // namespace runtime
