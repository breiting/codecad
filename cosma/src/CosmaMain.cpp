#include <CosmaMain.hpp>

#include "CosmaApplication.hpp"
#include "core/RenderEngine.hpp"

namespace CosmaMain {

void StartApp(std::shared_ptr<LuaEngine> coreEngine, const std::string& projectFile) {
    std::cout << "COSMA-MAIN" << std::endl;

    auto app = std::make_unique<CosmaApplication>(coreEngine);
    RenderEngine engine(std::move(app));
    if (!engine.Init(1200, 800, "CodeCAD Viewer")) {
        std::cerr << "Cannot initialize viewer" << std::endl;
    }

    engine.GetApp()->LoadProject(projectFile);
    engine.Run();
}

}  // namespace CosmaMain
