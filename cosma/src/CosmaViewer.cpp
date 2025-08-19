#include <cosma/CosmaController.hpp>
#include <cosma/CosmaViewer.hpp>
#include <iostream>

#include "CoreEngine.hpp"
#include "core/Engine.hpp"

CosmaViewer::CosmaViewer() {
}

void CosmaViewer::start(const std::string& projectFile, std::unique_ptr<CoreEngine> coreEngine) {
    auto app = std::make_unique<CosmaController>(std::move(coreEngine));
    Engine engine(std::move(app));
    if (!engine.Init(1200, 800, "CodeCAD Viewer")) {
        std::cerr << "Cannot initialize viewer" << std::endl;
    }

    engine.GetApp()->LoadProject(projectFile);

    engine.Run();
}
