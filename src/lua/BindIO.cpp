#include "runtime/BindIO.hpp"

#include "io/Export.hpp"
#include "runtime/LuaBindings.hpp"

namespace runtime {
void RegisterIO(sol::state& lua, LuaBindings* owner) {
    // emit/save
    lua.set_function("emit", [owner](const geo::ShapePtr& s) {
        owner->SetEmitted(s);
        return 0;
    });
    lua.set_function("save_stl", [](const geo::ShapePtr& s, const std::string& path) { io::SaveSTL(s, path, 0.1); });
    lua.set_function("save_step", [](const geo::ShapePtr& s, const std::string& path) { io::SaveSTEP(s, path); });
}
}  // namespace runtime
