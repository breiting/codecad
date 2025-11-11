#pragma once
#include <string>

#include "Controller.hpp"

class App {
   public:
    App();
    void start(int argc, char** argv);

   private:
    void handleNew(const std::string& name, const std::string& unit);
    void handlePartsAdd(const std::string& partName, const std::string& partMatName);
    void handleBuild(const std::string& rootDir);
    void handleLive(const std::string& rootDir);
    void handleParamsSet(const std::string& key, const std::string& value);
    void handleMaterialSet(const std::string& name, const std::string& color);
    void handleLspInit();
    void handleBom();
    void handleDoctor();

   private:
    std::unique_ptr<Controller> m_Controller;
};
