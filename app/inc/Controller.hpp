#pragma once
#include <RenderBridge.hpp>
#include <core/FileWatcher.hpp>
#include <core/LuaEngine.hpp>
#include <io/Project.hpp>
#include <memory>
#include <pure/PureController.hpp>

namespace fs = std::filesystem;

class Controller {
   public:
    Controller(std::vector<std::string>& luaPaths);
    void LoadProject(const fs::path& projectDir);
    void BuildProject();
    void ViewProject();
    void CreateBom();

    void HealthCheck();

   private:
    void SetupEngine();

    // void RebuildPart(const std::string& partId);
    // void OnProjectChanged();
    // void OnLuaChanged(const std::string& luaPath);

   private:
    io::Project m_Project;
    // std::unique_ptr<pure::PureController> pure;
    // std::unique_ptr<RenderBridge> bridge;
    //
    // FileWatcher projectWatch;
    // std::unordered_map<std::string, FileWatcher> luaWatchers;
    //

    std::shared_ptr<LuaEngine> m_Engine;
    std::vector<std::string> m_LuaPaths;

    std::string m_ProjectDir;
    bool m_ProjectLoaded = false;
};
