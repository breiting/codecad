#include "App.hpp"

#ifdef ENABLE_COSMA
#include "CosmaMain.hpp"
#endif
#include "core/LuaEngine.hpp"

App::App() {
}

void App::start(int argc, char** argv) {
    m_Engine = std::make_shared<LuaEngine>();

    // TODO: needs to be removed later!
    std::string libRoot = "/Users/breiting/workspace/codecad/lib";
    std::string path = libRoot + "/?.lua;" + libRoot + "/?/init.lua;";

    m_Engine->SetLibraryPaths({"./lib/?.lua", "./lib/?/init.lua", "./vendor/?.lua", "./vendor/?/init.lua", path});
    m_Engine->SetOutputDir("gen");
    std::string err;
    if (!m_Engine->Initialize(&err)) {
        throw std::runtime_error(std::string("CoreEngine init failed: ") + err);
    }

    std::string projectFile = "project.json";

#ifdef ENABLE_COSMA
    CosmaMain::StartApp(m_Engine, projectFile);
#else
    std::cerr << "This build does not support COSMA live viewer" << std::endl;
#endif
}
