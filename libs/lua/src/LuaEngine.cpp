#include "ccad/lua/LuaEngine.hpp"

#include <iostream>
#include <sstream>

#include "ccad/lua/Bindings.hpp"

namespace ccad::lua {

LuaEngine::LuaEngine() : m_Emitted(nullptr) {
}

LuaEngine::~LuaEngine() = default;

void LuaEngine::SetLibraryPaths(const std::vector<std::string>& paths) {
    m_LibraryPaths = paths;
}

std::string LuaEngine::BuildPackagePathPrefix() const {
    // Join entries with ';'
    std::ostringstream oss;
    for (size_t i = 0; i < m_LibraryPaths.size(); ++i) {
        oss << m_LibraryPaths[i];
        if (!m_LibraryPaths[i].empty() && m_LibraryPaths[i].back() != ';') {
            oss << ';';
        }
    }
    return oss.str();
}

bool LuaEngine::Initialize(std::string* errorMsg) {
    if (m_Initialized) return true;

    try {
        m_Lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::package, sol::lib::string,
                             sol::lib::io);

        if (!m_LibraryPaths.empty()) {
            std::string current = m_Lua["package"]["path"].get<std::string>();
            m_Lua["package"]["path"] = BuildPackagePathPrefix() + current;
        }

        m_Lua.create_named_table("PARAMS");

        m_Lua.set_function("mm", [](double v) { return v; });
        m_Lua.set_function("deg", [](double v) { return v; });

        m_Lua.set_function("param", [this](const std::string& name, sol::object def) -> sol::object {
            sol::table p = m_Lua["PARAMS"];
            sol::object v = p[name];

            // Wenn PARAMS[name] gesetzt ist (nicht nil/none), versuche ggf. Konvertierung
            if (v.valid() && v.get_type() != sol::type::lua_nil && v.get_type() != sol::type::none) {
                // Wenn def eine Zahl ist, aber v als String kommt -> parse Zahl
                if (def.is<double>() && v.is<std::string>()) {
                    try {
                        const std::string s = v.as<std::string>();
                        return sol::make_object(m_Lua, std::stod(s));
                    } catch (...) {
                        // Fallback: ungeparst zurückgeben
                        return v;
                    }
                }
                // Wenn def bool ist, String zu bool mappen
                if (def.is<bool>() && v.is<std::string>()) {
                    std::string s = v.as<std::string>();
                    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                    const bool b = (s == "1" || s == "true" || s == "yes" || s == "on");
                    return sol::make_object(m_Lua, b);
                }
                // Sonst: Wert einfach durchreichen (Zahl bleibt Zahl, Bool bleibt Bool, Tabelle bleibt Tabelle)
                return v;
            }
            // Nicht gesetzt → Default zurück
            return def;
        });

        // 4) Bindings registrieren
        RegisterPrimitives(m_Lua);
        RegisterIO(m_Lua, this);
        RegisterTransforms(m_Lua);
        RegisterBooleans(m_Lua);
        RegisterConstruct(m_Lua);
        RegisterFeatures(m_Lua);
        RegisterMeasure(m_Lua);
        // runtime::RegisterSketch(m_Lua);
        // runtime::RegisterPipes(m_Lua);
        // runtime::RegisterThreads(m_Lua);

        m_Initialized = true;
        return true;
    } catch (const std::exception& e) {
        if (errorMsg) *errorMsg = e.what();
        return false;
    } catch (...) {
        if (errorMsg) *errorMsg = "Unknown error during CoreEngine::Initialize()";
        return false;
    }
}

bool LuaEngine::RunFile(const std::string& scriptPath, std::string* errorMsg) {
    if (!m_Initialized) {
        if (errorMsg) *errorMsg = "CoreEngine not initialized";
        return false;
    }

    sol::load_result chunk = m_Lua.load_file(scriptPath);
    if (!chunk.valid()) {
        sol::error err = chunk;
        if (errorMsg) *errorMsg = std::string("Lua load error: ") + err.what();
        return false;
    }

    sol::protected_function_result result = chunk();
    if (!result.valid()) {
        sol::error err = result;
        if (errorMsg) *errorMsg = std::string("Lua runtime error: ") + err.what();
        return false;
    }
    return true;
}

bool LuaEngine::RunString(const std::string& script, std::string* errorMsg) {
    if (!m_Initialized) {
        if (errorMsg) *errorMsg = "CoreEngine not initialized";
        return false;
    }
    sol::load_result chunk = m_Lua.load(script.c_str());
    if (!chunk.valid()) {
        sol::error err = chunk;
        if (errorMsg) *errorMsg = std::string("Lua load error: ") + err.what();
        return false;
    }

    sol::protected_function_result result = chunk();
    if (!result.valid()) {
        sol::error err = result;
        if (errorMsg) *errorMsg = std::string("Lua runtime error: ") + err.what();
        return false;
    }
    return true;
}

void LuaEngine::Reset() {
    // Recreate state & bindings
    m_Lua = sol::state{};
    m_Initialized = false;
    // Re-init with previous config
    std::string err;
    if (!Initialize(&err)) {
        // If initialization fails here, leave it to caller to handle on next call.
        std::cerr << "CoreEngine::Reset() init failed: " << err << "\n";
    }
}

std::optional<Shape> LuaEngine::GetEmitted() const {
    return m_Emitted;
}

void LuaEngine::SetEmitted(const Shape& s) {
    m_Emitted = s;
}

geom::TriMesh LuaEngine::TriangulateEmitted() const {
    if (!m_Emitted) {
        throw std::runtime_error("TriangulateEmitted: no shape has been emitted");
    }

    return geom::Triangulate(m_Emitted, GetTriangulationParameters());
}

sol::state& LuaEngine::Lua() {
    return m_Lua;
}
}  // namespace ccad::lua
