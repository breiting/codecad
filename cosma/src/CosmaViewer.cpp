#include <cosma/CosmaViewer.hpp>
#include <iostream>

CosmaViewer::CosmaViewer() {
}

void CosmaViewer::start(const Project &project) {
    std::cout << "[CosmaViewer] Opening project: " << project.meta.name << std::endl;
}
