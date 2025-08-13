#pragma once
#include <runtime/SolConfig.hpp>

#include "geo/Shape.hpp"

namespace runtime {

class LuaBindings {
   public:
    LuaBindings();
    /**
     * @brief Registers functions into the given Lua state.
     *
     * This method registers C++ functions and types to be accessible from within Lua scripts.
     * @param lua The Lua state in which functions will be registered.
     */
    void Register(sol::state& lua);

    /**
     * @brief Sets the emitted shape model.
     *
     * This method stores the given shape as the emitted shape.
     * @param s The shape to set as emitted.
     */
    void SetEmitted(const geo::ShapePtr& s) {
        m_Emitted = s;
    }
    /**
     * @brief Retrieves the currently emitted shape.
     *
     * This method returns the shape that was previously set as emitted.
     * @return A pointer to the emitted shape.
     */
    geo::ShapePtr GetEmitted() const {
        return m_Emitted;
    }

   private:
    geo::ShapePtr m_Emitted;
};

}  // namespace runtime
