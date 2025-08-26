#pragma once

#include <memory>

#include "LuaEngine.hpp"

class App {
   public:
    App();
    void start(int argc, char** argv);

   private:
    void handleNew(const std::string& name, const std::string& unit, int workAreaWidth, int workAreaDepth);
    void handlePartsAdd(const std::string& partName);
    void handleBuild(const std::string& rootDir);
    void handleLive(const std::string& rootDir);
    void handleLspInit();
    void handleDoctor();

    void setupEngine();

   private:
    std::shared_ptr<LuaEngine> m_Engine;
    std::vector<std::string> m_LuaPaths;
};
