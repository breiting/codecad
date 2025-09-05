#pragma once
#include "sol/state.hpp"

class LuaEngine;

namespace ccad {
namespace lua {

void RegisterIO(sol::state& lua, LuaEngine* owner);
void RegisterPrimitives(sol::state& lua);
void RegisterTransforms(sol::state& lua);

}  // namespace lua
}  // namespace ccad
