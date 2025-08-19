#include "App.hpp"

#include <CLI/CLI.hpp>

#include "io/Export.hpp"
#include "io/Project.hpp"

#ifdef ENABLE_COSMA
#include "CosmaMain.hpp"
#endif
#include "core/LuaEngine.hpp"

namespace fs = std::filesystem;

App::App() : m_ProjectName("project.json"), m_GeneratedSubDir("generated") {
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
}

void App::start(int argc, char** argv) {
    CLI::App app{};

    app.description(R"(
CodeCAD - Parametric CAD in Code

CodeCAD is a lightweight parametric CAD tool where models are defined directly in Lua scripts.
It combines the flexibility of programming with the precision of CAD, enabling fast prototyping,
reusable components, and integration with 3D printing workflows.
	)");

    app.footer("https://github.com/breiting/codecad");

    // ---------- global options ----------
    app.add_option("--luapath", luaPaths, "Additional Lua search paths (repeatable)")->take_all();

    // ---------- subcommands ----------

    // new
    auto* cmdNew = app.add_subcommand("new", "Create a new project in the current folder");
    cmdNew->add_option("name", m_ProjectName, "Name of the project folder with the given name");
    cmdNew->callback([this]() { handleNew(); });

    // parts add "<Part Name>"
    auto* cmd_parts = app.add_subcommand("parts", "Manage parts");
    auto* cmd_add = cmd_parts->add_subcommand("add", "Add a new part to the current project (in CWD)");
    cmd_add->add_option("name", m_PartName, "Part name (e.g. \"Bracket A\")")->required();
    cmd_add->callback([this]() { handlePartsAdd(); });

    // live [<name>]
    auto* cmdLive = app.add_subcommand("live", "Start live viewer");
    cmdLive->add_option("name", m_ProjectName, "Project file name");
    cmdLive->callback([this]() { handleLive(); });

    // build [<name>]
    auto* cmdBuild = app.add_subcommand("build", "Generate STL files");
    cmdBuild->add_option("name", m_ProjectName, "Project file name");
    cmdBuild->callback([this]() { handleBuild(); });

    // ---------- parse & dispatch ----------
    try {
        app.require_subcommand(1);  // force one subcommand
        app.parse(argc, argv);      // runs callback of chosen subcommand
    } catch (const CLI::CallForHelp& e) {
        std::cout << app.help() << "\n";
        return;
    } catch (const CLI::ParseError& e) {
        std::exit(app.exit(e));
    }
}

// ---------------- handlers (stubs) ----------------

void App::handleNew() {
    // implement scaffold here
    std::cout << "Scaffold project at: " << m_ProjectName << "\n";
}

void App::handlePartsAdd() {
    // implement parts add here
    std::cout << "Add part: " << m_PartName << "\n";
}

void App::handleBuild() {
    auto project = io::LoadProject(m_ProjectName);
    io::PrintProject(project);
    auto projectRoot = std::filesystem::absolute(std::filesystem::path(m_ProjectName)).parent_path();

    auto outDir = projectRoot / m_GeneratedSubDir;
    fs::create_directory(outDir);

    for (const auto& jp : project.parts) {
        std::filesystem::path src = projectRoot / jp.source;
        auto luaFile = std::filesystem::weakly_canonical(src);

        std::string err;
        if (!m_Engine->RunFile(luaFile, &err)) {
            auto errStr = std::string("Lua error in ") + luaFile.string() + ": " + err;
            std::cerr << errStr << std::endl;
            return;
        }

        auto emitted = m_Engine->GetEmitted();
        if (!emitted) {
            std::cerr << "Canot get shape from " << luaFile << std::endl;
            return;
        }
        const auto stem = fs::path(luaFile).stem().string();
        const fs::path stlFile = outDir / (stem + ".stl");
        io::SaveSTL(emitted.value(), stlFile.string(), 0.1);
    }
}

void App::handleLive() {
#ifdef ENABLE_COSMA
    CosmaMain::StartApp(m_Engine, m_ProjectName);
#else
    std::cerr << "This build does not support COSMA live viewer" << std::endl;
#endif
}

// // parts add <name>
// cmdParts = m_App.add_subcommand("parts", "Project part organization");
// cmdPartsAdd = cmdParts->add_subcommand("add", "Add a new part");
// cmdPartsAdd->add_option("name", m_PartName, "Name of the part")->required();
//
// // build [<name>]
// cmdBuild = m_App.add_subcommand("build", "Build the project");
// cmdBuild->add_option("name", m_ProjectName, "Project file name");
// cmdBuild->add_option("subdir", m_GeneratedSubDir, "Subdir for generated files, (default=generated)");
//
// // live [<name>]
// cmdLive = m_App.add_subcommand("live", "Start live viewer");
// cmdLive->add_option("name", m_ProjectName, "Project file name");
// }
//
// void App::start(int argc, char** argv) {
//     try {
//         m_App.parse(argc, argv);
//     } catch (const CLI::ParseError& e) {
//         std::exit(m_App.exit(e));
//     }
//
// void App::handleNew() {
//     if (m_ProjectName.empty()) {
//         // current directory
//         std::ofstream("project.json") << "{ \"name\": \"default\" }\n";
//         std::cout << "Projekt im aktuellen Verzeichnis erstellt.\n";
//     } else {
//         fs::create_directories(m_ProjectName);
//         std::ofstream(fs::path(m_ProjectName) / "project.json") << "{ \"name\": \"" << m_ProjectName << "\" }\n";
//         std::cout << "Projekt '" << m_ProjectName << "' erstellt.\n";
//     }
// }
//
// void App::handlePartsAdd() {
//     fs::create_directories("parts");
//     std::ofstream("project.json", std::ios::app) << "// Added part: " << m_PartName << "\n";
//     std::ofstream(fs::path("parts") / (m_PartName + ".lua")) << "-- Lua script for part " << m_PartName << "\n";
//     std::cout << "Part '" << m_PartName << "' hinzugefügt.\n";
// }
//
// void App::handleBuild() {
//     std::string name = m_ProjectName.empty() ? "current" : m_ProjectName;
// }
