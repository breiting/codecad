#pragma once
#include <runtime/SolConfig.hpp>

#include "geo/Shape.hpp"

namespace runtime {

class LuaBindings {
   public:
    LuaBindings();
    // Registers functions into the given Lua state.
    void Register(sol::state& lua);

    // Storage for the "emitted" model.
    void SetEmitted(const geo::ShapePtr& s) {
        m_Emitted = s;
    }
    geo::ShapePtr GetEmitted() const {
        return m_Emitted;
    }

   private:
    geo::ShapePtr m_Emitted;
};

}  // namespace runtime
