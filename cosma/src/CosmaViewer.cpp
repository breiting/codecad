#include <cosma/CosmaViewer.hpp>
#include <iostream>

CosmaViewer::CosmaViewer() {
}

void CosmaViewer::start(const std::string& projectFile) {
    std::cout << "[CosmaViewer] Opening project: " << projectFile << std::endl;
}
