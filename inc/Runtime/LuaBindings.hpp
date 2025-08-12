#pragma once
#include <Runtime/SolConfig.hpp>

#include "Geo/Shape.hpp"

namespace Runtime {

class LuaBindings {
   public:
    LuaBindings();
    // Registers functions into the given Lua state.
    void Register(sol::state& lua);

    // Storage for the "emitted" model.
    void SetEmitted(const Geo::ShapePtr& s) {
        m_Emitted = s;
    }
    Geo::ShapePtr GetEmitted() const {
        return m_Emitted;
    }

   private:
    Geo::ShapePtr m_Emitted;
};

}  // namespace Runtime
