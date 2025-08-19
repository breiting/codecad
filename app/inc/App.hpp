#pragma once

#include <memory>

#include "LuaEngine.hpp"

class App {
   public:
    App();
    void start(int argc, char** argv);

    // global options (populated by CLI)
    std::vector<std::string> luaPaths;

    // subcommand options
    std::string m_ProjectName;
    std::string m_PartName;
    std::string m_GeneratedSubDir;

    void handleNew();
    void handlePartsAdd();
    void handleBuild();
    void handleLive();

   private:
    std::shared_ptr<LuaEngine> m_Engine;
};
