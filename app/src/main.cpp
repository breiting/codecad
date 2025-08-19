#include <core/Viewer.hpp>
#include <iostream>

#ifdef ENABLE_COSMA
#include "cosma/CosmaViewer.hpp"
#endif

std::unique_ptr<Viewer> createViewer() {
#ifdef ENABLE_COSMA
    return std::make_unique<CosmaViewer>();
#else
    return nullptr;
#endif
}

namespace fs = std::filesystem;

struct Cmd {
    std::string command;
    std::string projectFile;
    fs::path outDir;
};

static void PrintUsage(const std::string& name) {
    std::cout << "Usage: " << name << " <command> project.json\n";
}

static bool ParseArgs(int argc, char** argv, Cmd& cmd) {
    if (argc < 3) return false;
    cmd.command = argv[1];
    cmd.projectFile = argv[2];
    return cmd.command == "build" || cmd.command == "live";
}

int main(int argc, char** argv) {
    Cmd cmd;
    if (!ParseArgs(argc, argv, cmd)) {
        PrintUsage(argv[0]);
        return 1;
    }

    // TODO: should be a parameter
    cmd.outDir = "out";
    fs::create_directory(cmd.outDir);

    // load project
    // Project project;
    // try {
    //     project = LoadProject(cmd.projectFile);
    // } catch (std::exception e) {
    //     std::cerr << e.what() << std::endl;
    // }

    // core::Core coreEngine;
    // coreEngine.load(projectFile);

    if (cmd.command == "live") {
        auto viewer = createViewer();
        if (viewer) {
            viewer->start(cmd.projectFile, cmd.outDir);
        } else {
            std::cout << "Viewer support is not available in this build.\n";
        }
    } else if (cmd.command == "build") {
    }

    return 0;
}
