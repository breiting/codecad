#pragma once
#include <runtime/SolConfig.hpp>
class CoreEngine;

namespace runtime {

/// \brief RegisterIO
void RegisterIO(sol::state& lua, CoreEngine* owner);

}  // namespace runtime
