#include <iostream>
#include <pure/PureController.hpp>

using namespace pure;

int main() {
    PureController controller;
    if (!controller.Initialize(1280, 800, "PURE Viewer")) {
        std::cerr << "Failed to initialize PURE\n";
        return 1;
    }
    controller.Run();
    controller.Shutdown();
    return 0;
}
