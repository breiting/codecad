#include "App.hpp"

#include <CLI/CLI.hpp>
#include <filesystem>

#include "assets/part_template.h"
#include "assets/readme_template.h"
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

static std::string Slugify(const std::string& name) {
    std::string s;
    s.reserve(name.size());
    for (unsigned char c : name) {
        if (std::isalnum(c))
            s.push_back(std::tolower(c));
        else
            s.push_back('_');
    }
    // collapse multiple underscores
    auto end = std::unique(s.begin(), s.end(), [](char a, char b) { return a == '_' && b == '_'; });
    s.erase(end, s.end());
    if (!s.empty() && s.front() == '_') s.erase(s.begin());
    if (!s.empty() && s.back() == '_') s.pop_back();
    if (s.empty()) s = "part";
    return s;
}

App::App() = default;

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
    app.add_option("--luapath", m_LuaPaths, "Additional Lua search paths")->take_all();

    // ---------- subcommands ----------

    // new
    std::string projectName = "My CodeCAD Project";
    std::string units = "mm";
    // Prusa Core One Bed
    int workAreaWidth = 220;
    int workAreaDepth = 250;
    auto* cmdInit = app.add_subcommand("init", "Initializes a new project structure in the current directory");
    cmdInit->add_option("--name", projectName, "Name of the project");
    cmdInit->add_option("--units", units, "Units: mm, cm, m")->capture_default_str();
    cmdInit->add_option("--wa-w", workAreaWidth, "Workarea width [mm]")->capture_default_str();
    cmdInit->add_option("--wa-d", workAreaDepth, "Workarea depth [mm]")->capture_default_str();

    // parts add "<Part Name>"
    std::string partName = "Part 1";
    auto* cmdParts = app.add_subcommand("parts", "Manage parts");
    auto* cmdAdd = cmdParts->add_subcommand("add", "Add a new part to the project");
    cmdAdd->add_option("--name", partName, "Part name (e.g. \"Bracket A\")")->required();

    // live [<rootDir>]
    std::string liveRoot = ".";
    auto* cmdLive = app.add_subcommand("live", "Start live viewer");
    cmdLive->add_option("--root", liveRoot, "Project directory");

    // build [<rootDir>]
    std::string buildRoot = ".";
    auto* cmdBuild = app.add_subcommand("build", "Generate STL files");
    cmdBuild->add_option("--root", buildRoot, "Project directory");

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

    // setup the lua engine
    setupEngine();

    // Dispatch manually
    if (*cmdLive) {
        handleLive(liveRoot);
        return;
    }
    if (*cmdBuild) {
        handleBuild(buildRoot);
        return;
    }
    if (*cmdParts && *cmdAdd) {
        handlePartsAdd(partName);
        return;
    }
    if (*cmdInit) {
        handleNew(projectName, units, workAreaWidth, workAreaDepth);
        return;
    }

    // Show help if nothing was executed
    std::cout << app.help() << "\n";
}

void App::setupEngine() {
    m_Engine = std::make_shared<LuaEngine>();

    // Standard search paths
    std::vector<std::string> paths = {"./lib/?.lua", "./lib/?/init.lua", "./vendor/?.lua", "./vendor/?/init.lua"};

    // Environment-Variable LUA_PATH (optional)
    if (const char* env = std::getenv("LUA_PATH")) {
        std::stringstream ss(env);
        std::string token;
        while (std::getline(ss, token, ';')) {
            if (!token.empty()) paths.push_back(token);
        }
    }

    // CLI options
    paths.insert(paths.end(), m_LuaPaths.begin(), m_LuaPaths.end());
    m_Engine->SetLibraryPaths(paths);

    std::string err;
    if (!m_Engine->Initialize(&err)) {
        throw std::runtime_error(std::string("CoreEngine init failed: ") + err);
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

    const std::string readme = "# " + projectName + "\n";
    std::ofstream(rootDir / "README.md") << readme << readme_template;

    std::ofstream(rootDir / ".gitignore") << PROJECT_OUTDIR << "/\n";

    std::cout << "Project scaffold created:\n"
              << "  " << fs::absolute(rootDir) << "\n"
              << "  - project.json\n"
              << "  - parts/\n"
              << "  - " << PROJECT_OUTDIR << "/\n\n"
              << "Next steps:\n"
              << "  1) codecad parts add --name \"Part 1\"\n"
              << "  2) codecad live\n"
              << "  3) codecad build\n";
}

void App::handlePartsAdd(const std::string& partName) {
    try {
        const fs::path pj = fs::current_path() / PROJECT_FILENAME;
        if (!fs::exists(pj)) {
            std::cerr << "Error: project.json not found in current directory. "
                         "Run `init` command first or cd into a project.\n";
            return;
        }

        io::Project p;
        try {
            p = io::LoadProject(pj.string());
        } catch (const std::exception& e) {
            std::cerr << "Error: failed to load project.json: " << e.what() << "\n";
            return;
        }

        fs::path partsDir = fs::current_path() / PARTS_DIR;
        fs::create_directories(partsDir);

        std::string slug = Slugify(partName);
        fs::path luaPath = partsDir / (slug + ".lua");
        int suffix = 1;
        while (fs::exists(luaPath)) {
            luaPath = partsDir / (slug + "_" + std::to_string(suffix++) + ".lua");
        }
        const std::string finalFile = luaPath.filename().string();

        // Get content from template file
        std::string tpl = std::string(part_template);
        std::ofstream(luaPath) << tpl;

        // Add part in project file
        io::Part pr{};
        pr.id = "";  // optional
        pr.name = partName;
        pr.source = std::string(PARTS_DIR) + "/" + finalFile;
        pr.material = "";  // optional: default material key
        pr.transform.translate = {0, 0, 0};
        pr.transform.rotate = {0, 0, 0};
        pr.transform.scale = 1;
        pr.visible = true;
        p.parts.push_back(std::move(pr));

        // Save project
        if (!io::SaveProject(p, pj.string(), /*pretty*/ true)) {
            std::cerr << "Error: failed to save " << pj << "\n";
            return;
        }

        std::cout << "Added part: " << partName << "\n"
                  << "  file: " << (fs::path("parts") / finalFile) << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Fatal in handlePartsAdd: " << e.what() << "\n";
    }
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
