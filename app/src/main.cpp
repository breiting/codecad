#include <core/Viewer.hpp>
#include <iostream>

#include "core/CoreEngine.hpp"
#include "core/io/Export.hpp"
#include "io/Project.hpp"

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

static bool BuildProject(const std::string& projectFile, std::unique_ptr<CoreEngine> engine,
                         const fs::path& projectOutputPath) {
    auto project = io::LoadProject(projectFile);
    io::PrintProject(project);
    auto projectRoot = std::filesystem::absolute(std::filesystem::path(projectFile)).parent_path();

    auto outDir = projectRoot / projectOutputPath;
    fs::create_directory(outDir);

    for (const auto& jp : project.parts) {
        std::filesystem::path src = projectRoot / jp.source;
        auto luaFile = std::filesystem::weakly_canonical(src);

        std::string err;
        if (!engine->RunFile(luaFile, &err)) {
            auto errStr = std::string("Lua error in ") + luaFile.string() + ": " + err;
            std::cerr << errStr << std::endl;
            return false;
        }

        auto emitted = engine->GetEmitted();
        if (!emitted) {
            std::cerr << "Canot get shape from " << luaFile << std::endl;
            return false;
        }
        const auto stem = fs::path(luaFile).stem().string();
        const fs::path stlFile = outDir / (stem + ".stl");
        io::SaveSTL(emitted.value(), stlFile.string(), 0.1);
    }
    return true;
}

int main(int argc, char** argv) {
    Cmd cmd;
    if (!ParseArgs(argc, argv, cmd)) {
        PrintUsage(argv[0]);
        return 1;
    }

    cmd.outDir = "generated";  // as parameter?

    std::string libRoot = "/Users/breiting/workspace/codecad/lib";
    std::string path = libRoot + "/?.lua;" + libRoot + "/?/init.lua;";

    auto engine = std::make_unique<CoreEngine>();
    engine->SetLibraryPaths({"./lib/?.lua", "./lib/?/init.lua", "./vendor/?.lua", "./vendor/?/init.lua", path});

    engine->SetOutputDir(cmd.outDir);
    std::string err;
    if (!engine->Initialize(&err)) {
        throw std::runtime_error(std::string("CoreEngine init failed: ") + err);
    }

    if (cmd.command == "live") {
        auto viewer = createViewer();
        if (viewer) {
            viewer->start(cmd.projectFile, std::move(engine));
        } else {
            std::cerr << "Viewer support is not available in this build.\n";
        }
    } else if (cmd.command == "build") {
        if (!BuildProject(cmd.projectFile, std::move(engine), cmd.outDir)) {
            std::cerr << "Error during file generation." << std::endl;
        }
    }

    return 0;
}
