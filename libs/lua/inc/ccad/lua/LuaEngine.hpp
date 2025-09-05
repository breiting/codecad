#pragma once
#include <ccad/base/Shape.hpp>
#include <ccad/geom/Triangulation.hpp>
#include <sol/sol.hpp>
#include <string>

namespace ccad {
namespace lua {

static inline geom::TriangulationParams GetTriangulationParameters() {
    geom::TriangulationParams params;
    params.angularDeflectionDeg = 25.0;
    params.linearDeflection = 0.1;
    params.parallel = true;
    return params;
}

/**
 * @brief Thin wrapper around a sol2 Lua state + CodeCAD bindings.
 *
 * Responsibilities:
 * - Open Lua libs, prepend library paths to package.path
 * - Register CodeCAD Lua bindings once
 * - Execute a Lua file (protected), capture errors
 * - Expose emitted shape (from bindings) and optional triangulation helper
 */
class LuaEngine {
   public:
    LuaEngine();
    ~LuaEngine();

    /// Configure library search paths to prepend to package.path.
    /// Example entries: "./lib/?.lua", "./lib/?/init.lua", "./vendor/?.lua"
    void SetLibraryPaths(const std::vector<std::string>& paths);

    /// Initialize Lua VM and register bindings. Idempotent.
    /// Returns false on failure and fills errorMsg.
    bool Initialize(std::string* errorMsg = nullptr);

    /// Execute a Lua script file (path). Initialize() must be called first.
    /// Returns false on error and fills errorMsg with a descriptive message.
    bool RunFile(const std::string& scriptPath, std::string* errorMsg = nullptr);

    /// Execute a Lua script. Initialize() must be called first.
    /// Returns false on error and fills errorMsg with a descriptive message.
    bool RunString(const std::string& script, std::string* errorMsg = nullptr);

    /// Reset the engine to a clean state (fresh Lua, fresh bindings).
    void Reset();

    /// Access emitted shape (if any) as produced by Lua 'emit(...)'.
    std::optional<Shape> GetEmitted() const;

    /// Set the emitted shape
    void SetEmitted(const Shape& s);

    /// Triangulate the emitted shape for real-time viewing. Throws if no shape.
    geom::TriMesh TriangulateEmitted() const;

    /// Direct access to the Lua state if advanced users need it.
    sol::state& Lua();

   private:
    /// Build the package.path prefix string from m_LibraryPaths.
    std::string BuildPackagePathPrefix() const;

   private:
    sol::state m_Lua;
    std::vector<std::string> m_LibraryPaths;
    Shape m_Emitted;

    bool m_Initialized{false};
};
}  // namespace lua
}  // namespace ccad
