#pragma once
#include "sol/state.hpp"

class LuaEngine;

namespace ccad {
namespace lua {

void RegisterIO(sol::state& lua, LuaEngine* owner);
void RegisterPrimitives(sol::state& lua);
void RegisterTransforms(sol::state& lua);
void RegisterBooleans(sol::state& lua);
void RegisterConstruct(sol::state& lua);
void RegisterFeatures(sol::state& lua);
void RegisterMeasure(sol::state& lua);
void RegisterSelect(sol::state& lua);
void RegisterSketch(sol::state& lua);
void RegisterMech(sol::state& lua);
void RegisterCurves(sol::state& lua);

}  // namespace lua
}  // namespace ccad
