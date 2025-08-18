#include <cosma/CosmaController.hpp>
#include <cosma/CosmaViewer.hpp>
#include <iostream>

#include "core/Engine.hpp"

CosmaViewer::CosmaViewer() {
}

void CosmaViewer::start(const Project &project) {
    auto app = std::make_unique<CosmaController>();
    Engine engine(std::move(app));
    if (!engine.Init(1200, 800, "CodeCAD Viewer")) {
        std::cerr << "Cannot initialize viewer" << std::endl;
    }

    engine.GetApp()->LoadProject(project);

    engine.Run();
}
