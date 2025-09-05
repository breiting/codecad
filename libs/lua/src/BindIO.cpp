#include <ccad/io/Export.hpp>
#include <sol/sol.hpp>

#include "ccad/lua/LuaEngine.hpp"

namespace ccad {
namespace lua {
void RegisterIO(sol::state& lua, LuaEngine* owner) {
    // emit
    lua.set_function("emit", [owner](const Shape& s) {
        owner->SetEmitted(s);
        return 0;
    });
    lua.set_function("save_stl", [](const Shape& s, const std::string& path) {
        io::SaveSTL(s, path, GetTriangulationParameters());
    });
    lua.set_function("save_step", [](const Shape& s, const std::string& path) { io::SaveSTEP(s, path); });
}
}  // namespace lua
}  // namespace ccad
