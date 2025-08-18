#include <core/Viewer.hpp>
#include <iostream>

#ifdef ENABLE_COSMA
#include "cosma/CosmaViewer.hpp"
#endif

std::unique_ptr<Viewer> Viewer::create() {
#ifdef ENABLE_COSMA
    return std::make_unique<CosmaViewer>();
#else
    return nullptr;
#endif
}

int main(int argc, char** argv) {
    std::string projectFile = (argc > 1) ? argv[1] : "project.json";

    // core::Core coreEngine;
    // coreEngine.load(projectFile);

    auto viewer = Viewer::create();
    if (viewer) {
        viewer->start(projectFile);
    } else {
        std::cout << "Viewer support is not available in this build.\n";
    }

    return 0;
}
