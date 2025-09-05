#pragma once
#include <ccad/base/Math.hpp>
#include <vector>

#include "sol/state.hpp"

namespace ccad {
namespace lua {

/// Parse a Lua table of points into vector<pair<double,double>>.
/// Accepts {{x,y},...} or {{x=...,y=...},...}. Throws std::runtime_error on format errors.
std::vector<Vec2> ParsePointTable(sol::table t);
}  // namespace lua
}  // namespace ccad
