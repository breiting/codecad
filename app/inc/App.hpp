#pragma once

#include <memory>

#include "LuaEngine.hpp"

class App {
   public:
    App();
    void start(int argc, char** argv);

    // global options (populated by CLI)
    std::vector<std::string> luaPaths;

    void handleNew(const std::string& name, const std::string& unit, int workAreaWidth, int workAreaDepth);
    void handlePartsAdd(const std::string& partName);
    void handleBuild(const std::string& rootDir);
    void handleLive(const std::string& rootDir);

   private:
    std::shared_ptr<LuaEngine> m_Engine;
};
