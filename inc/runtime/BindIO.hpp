#pragma once
#include <runtime/SolConfig.hpp>
namespace runtime {
class LuaBindings;

/// \brief RegisterIO
void RegisterIO(sol::state& lua, LuaBindings* owner);

}  // namespace runtime
