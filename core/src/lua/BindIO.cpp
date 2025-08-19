
#include "CoreEngine.hpp"
#include "io/Export.hpp"

namespace runtime {
void RegisterIO(sol::state& lua, CoreEngine* owner) {
    // emit
    lua.set_function("emit", [owner](const geometry::ShapePtr& s) {
        owner->SetEmitted(s);
        return 0;
    });
    lua.set_function("save_stl",
                     [](const geometry::ShapePtr& s, const std::string& path) { io::SaveSTL(s, path, 0.1); });
    lua.set_function("save_step", [](const geometry::ShapePtr& s, const std::string& path) { io::SaveSTEP(s, path); });
}

}  // namespace runtime
