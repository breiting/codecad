#pragma once
#include <runtime/SolConfig.hpp>
#include <utility>
#include <vector>

namespace runtime {

/// Parse a Lua table of points into vector<pair<double,double>>.
/// Accepts {{x,y},...} or {{x=...,y=...},...}. Throws std::runtime_error on format errors.
std::vector<std::pair<double, double>> ParsePointTable(sol::table t);

}  // namespace runtime
