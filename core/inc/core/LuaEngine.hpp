#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "geometry/Shape.hpp"
#include "geometry/Triangulate.hpp"
#include "runtime/SolConfig.hpp"

/**
 * @brief Thin wrapper around a sol2 Lua state + CodeCAD bindings.
 *
 * Responsibilities:
 * - Open Lua libs, prepend library paths to package.path
 * - Register CodeCAD Lua bindings once
 * - Provide __OUTDIR to Lua
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

    /// Configure output directory; exposed to Lua as global __OUTDIR.
    void SetOutputDir(const std::filesystem::path& outdir);

    /// Initialize Lua VM and register bindings. Idempotent.
    /// Returns false on failure and fills errorMsg.
    bool Initialize(std::string* errorMsg = nullptr);

    /// Execute a Lua script file (path). Initialize() must be called first.
    /// Returns false on error and fills errorMsg with a descriptive message.
    bool RunFile(const std::string& scriptPath, std::string* errorMsg = nullptr);

    /// Reset the engine to a clean state (fresh Lua, fresh bindings).
    void Reset();

    /// Access emitted shape (if any) as produced by Lua 'emit(...)'.
    std::optional<geometry::ShapePtr> GetEmitted() const;

    /// Set the emitted shape
    void SetEmitted(const geometry::ShapePtr& s);

    /// Triangulate the emitted shape for real-time viewing. Throws if no shape.
    geometry::TriMesh TriangulateEmitted(double defl = 0.2, double angDeg = 20.0, bool parallel = true) const;

    /// Direct access to the Lua state if advanced users need it.
    sol::state& Lua();

   private:
    /// Build the package.path prefix string from m_LibraryPaths.
    std::string BuildPackagePathPrefix() const;

   private:
    sol::state m_Lua;
    std::vector<std::string> m_LibraryPaths;
    std::filesystem::path m_Outdir;
    geometry::ShapePtr m_Emitted;

    bool m_Initialized{false};
};
