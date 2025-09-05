#pragma once
#include <ccad/lua/LuaEngine.hpp>
#include <memory>
#include <pure/PureController.hpp>

#include "FileWatcher.hpp"
#include "Project.hpp"

namespace fs = std::filesystem;

class Controller {
   public:
    Controller(std::vector<std::string>& luaPaths);
    void LoadProject(const fs::path& projectDir);
    void BuildProject();
    void ViewProject();

    void HealthCheck();

   private:
    void SetupEngine();

    // --- Watcher lifecycle ---
    void SetupWatchers();
    void ResetLuaWatchers();
    void PollWatchers();

    // --- Watcher handlers ---
    void OnProjectChanged();
    void OnLuaChanged(const std::string& luaPath);

    // --- Build helpers ---
    void RebuildAllParts();
    void RebuildPartByPath(const std::string& luaPath);

    // --- Scene utils ---
    void ClearScene();
    void AddPartToScene(const Part& part);
    static std::string NormalizePath(const std::string& p);

   private:
    Project m_Project;
    std::string m_ProjectDir;
    bool m_ProjectLoaded = false;

    std::shared_ptr<pure::PureScene> m_Scene;
    pure::PureController m_PureController;

    // Watchers
    FileWatcher m_ProjectWatcher;
    std::unordered_map<std::string, FileWatcher> m_LuaWatchers;

    // Mapping: luaPath -> part.id
    std::unordered_map<std::string, std::string> m_LuaToPartId;

    // Debounce
    std::chrono::steady_clock::time_point m_LastProjectEvent{};
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> m_LastLuaEvent;
    int m_DebounceMs = 200;

    std::shared_ptr<ccad::lua::LuaEngine> m_Engine;
    std::vector<std::string> m_LuaPaths;
};
