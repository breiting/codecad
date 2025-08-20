#pragma once
#include <runtime/SolConfig.hpp>
class LuaEngine;

namespace runtime {

/// \brief RegisterIO
void RegisterIO(sol::state& lua, LuaEngine* owner);

}  // namespace runtime
