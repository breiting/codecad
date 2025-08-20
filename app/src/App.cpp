#include "App.hpp"

#include <CLI/CLI.hpp>
#include <filesystem>

#include "io/Export.hpp"
#include "io/Project.hpp"

#ifdef ENABLE_COSMA
#include "CosmaMain.hpp"
#endif
#include "core/LuaEngine.hpp"

const std::string PROJECT_FILENAME = "project.json";
const std::string PROJECT_OUTDIR = "generated";
const std::string PARTS_DIR = "parts";

namespace fs = std::filesystem;
using namespace std;

App::App() {
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
    std::string projectName = "My CodeCAD Project";
    std::string units = "mm";
    int workAreaWidth = 200;
    int workAreaDepth = 200;
    auto* cmdNew = app.add_subcommand("new", "Creates a new project structure in the current directory");
    cmdNew->add_option("name", projectName, "Name of the project");
    cmdNew->add_option("units", units, "Units: mm, cm, m")->capture_default_str();
    cmdNew->add_option("wa-w", workAreaWidth, "Workarea width [mm]")->capture_default_str();
    cmdNew->add_option("wa-d", workAreaDepth, "Workarea depth [mm]")->capture_default_str();
    cmdNew->callback([&, this]() { handleNew(projectName, units, workAreaWidth, workAreaDepth); });

    // parts add "<Part Name>"
    std::string partName = "Part 1";
    auto* cmdParts = app.add_subcommand("parts", "Manage parts");
    auto* cmdAdd = cmdParts->add_subcommand("add", "Add a new part to the project");
    cmdAdd->add_option("name", partName, "Part name (e.g. \"Bracket A\")")->required();
    cmdAdd->callback([&, this]() { handlePartsAdd(partName); });

    // live [<rootDir>]
    std::string rootDir = ".";
    auto* cmdLive = app.add_subcommand("live", "Start live viewer");
    cmdLive->add_option("--root", rootDir, "Project directory");
    cmdLive->callback([&, this]() { handleLive(rootDir); });

    // build [<rootDir>]
    auto* cmdBuild = app.add_subcommand("build", "Generate STL files");
    cmdBuild->add_option("--root", rootDir, "Project directory");
    cmdBuild->callback([&, this]() { handleBuild(rootDir); });

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

static bool EnsureDirs(const fs::path& root) {
    std::error_code ec;
    fs::create_directories(root / PARTS_DIR, ec);
    fs::create_directories(root / PROJECT_OUTDIR, ec);
    return !ec;
}

// ---------------- handlers (stubs) ----------------

void App::handleNew(const std::string& projectName, const std::string& unit, int workAreaWidth, int workAreaDepth) {
    auto rootDir = fs::current_path();

    if (!EnsureDirs(rootDir)) {
        std::cerr << "Error: Could not create directories under " << rootDir << "\n";
        return;
    }

    io::Project p;
    p.version = 1;
    p.meta.name = projectName;
    p.meta.author = "";
    p.meta.units = unit;
    p.workarea.size = {workAreaWidth, workAreaDepth};

    const fs::path pj = rootDir / PROJECT_FILENAME;
    if (!io::SaveProject(p, pj.string(), /*pretty*/ true)) {
        std::cerr << "Error: failed to write " << pj << "\n";
        return;
    }

    const std::string readme = "# " + projectName + R"(

This is a CodeCAD project. Start live mode:"

```bash
codecad live
```

Add a part:

```bash
codecad parts add "My Part"
```

Build STL files:

```bash
codecad build
```)";

    std::ofstream(rootDir / "README.md") << readme;

    std::ofstream(rootDir / ".gitignore") << PROJECT_OUTDIR << "/\n";

    std::cout << "Project scaffold created:\n"
              << "  " << fs::absolute(rootDir) << "\n"
              << "  - project.json\n"
              << "  - parts/\n"
              << "  - " << PROJECT_OUTDIR << "/\n\n"
              << "Next steps:\n"
              << "  1) codecad parts add \"Part 1\"\n"
              << "  2) codecad live\n"
              << "  3) codecad build\n";
}

void App::handlePartsAdd(const std::string& partName) {
    // implement parts add here
    std::cout << "Add part: " << partName << "\n";
}

void App::handleBuild(const std::string& rootDir) {
    auto projectFile = fs::path(rootDir) / PROJECT_FILENAME;
    auto project = io::LoadProject(projectFile);
    io::PrintProject(project);
    auto projectRoot = std::filesystem::absolute(std::filesystem::path(projectFile)).parent_path();

    auto outDir = projectRoot / PROJECT_OUTDIR;
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

/// Start COSMA viewer, if supported
void App::handleLive(const std::string& rootDir) {
    fs::path projectFile = fs::path(rootDir) / PROJECT_FILENAME;
#ifdef ENABLE_COSMA
    CosmaMain::StartApp(m_Engine, projectFile);
#else
    std::cerr << "This build does not support COSMA live viewer" << std::endl;
#endif
}
