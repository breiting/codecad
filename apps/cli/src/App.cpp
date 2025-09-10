#include "App.hpp"

#include <CLI/CLI.hpp>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <sstream>

#include "Controller.hpp"
#include "Project.hpp"
#include "Utils.hpp"
#include "assets/part_template.h"
#include "assets/readme_template.h"

const std::string PROJECT_FILENAME = "project.json";
const std::string PROJECT_OUTDIR = "generated";
const std::string PARTS_DIR = "parts";

namespace fs = std::filesystem;
using json = nlohmann::json;
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
    std::vector<std::string> luaPaths;
    app.add_option("--luapath", luaPaths, "Additional Lua search paths")->take_all();

    // ---------- subcommands ----------

    // new
    std::string projectName = "My CodeCAD Project";
    std::string units = "mm";
    auto* cmdInit = app.add_subcommand("init", "Initializes a new project structure in the current directory");
    cmdInit->add_option("name", projectName, "Name of the project");
    cmdInit->add_option("units", units, "Units (mm, cm, m)")->capture_default_str();

    // parts add [name <Part Name>]
    std::string partName = "part";
    std::string partMatName = "";
    auto* cmdParts = app.add_subcommand("parts", "Handle project parts");
    auto* cmdAdd = cmdParts->add_subcommand("add", "Add a new part to the project");
    cmdAdd->add_option("name", partName, "Part name (e.g. \"Bracket A\")");
    cmdAdd->add_option("mat", partMatName, "Material for the part (material must be added before)");

    // live [<rootDir>]
    std::string liveRoot = ".";
    auto* cmdLive = app.add_subcommand("live", "Start live viewer");
    cmdLive->add_option("root", liveRoot, "Project directory");

    // build [<rootDir>]
    std::string buildRoot = ".";
    auto* cmdBuild = app.add_subcommand("build", "Generate STL files");
    cmdBuild->add_option("root", buildRoot, "Project directory");

    // params set key <key> value <value>
    auto* cmdParams = app.add_subcommand("params", "Handle project parameters");
    auto* cmdSet = cmdParams->add_subcommand("set", "Set a param (bool/number/string)");
    std::string key, value;
    cmdSet->add_option("key", key, "Param key")->required();
    cmdSet->add_option("value", value, "Value (true/false/number/string)")->required();

    // material set key <key> value <value>
    auto* cmdMaterial = app.add_subcommand("mat", "Handle materials");
    auto* cmdMatSet = cmdMaterial->add_subcommand("set", "Set a material with name and hex color");
    std::string matName, matColor;
    cmdMatSet->add_option("name", matName, "Material name")->required();
    cmdMatSet->add_option("color", matColor, "Color in hex (e.g. #ff0000 for red)")->required();

    // bom
    auto* cmdBom = app.add_subcommand("bom", "Generate project cutlist/BOM");

    // lsp init
    auto* cmdLsp = app.add_subcommand("lsp", "Write LSP project file (.luarc.json)");

    // doctor
    auto* cmdDoctor = app.add_subcommand("doctor", "Diagnose your CodeCAD installation");

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

    // Setup controller
    m_Controller = std::make_unique<Controller>(luaPaths);

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
        handlePartsAdd(partName, partMatName);
        return;
    }
    if (*cmdInit) {
        handleNew(projectName, units);
        return;
    }
    if (*cmdParams && *cmdSet) {
        handleParamsSet(key, value);
        return;
    }
    if (*cmdMaterial && *cmdMatSet) {
        handleMaterialSet(matName, matColor);
        return;
    }
    if (*cmdBom) {
        handleBom();
        return;
    }
    if (*cmdLsp) {
        handleLspInit();
        return;
    }
    if (*cmdDoctor) {
        handleDoctor();
        return;
    }

    // Show help if nothing was executed
    std::cout << app.help() << "\n";
}

static bool EnsureDirs(const fs::path& root) {
    std::error_code ec;
    fs::create_directories(root / PARTS_DIR, ec);
    fs::create_directories(root / PROJECT_OUTDIR, ec);
    return !ec;
}

// ---------------- handlers (stubs) ----------------

void App::handleNew(const std::string& projectName, const std::string& unit) {
    auto rootDir = fs::current_path();

    if (!EnsureDirs(rootDir)) {
        std::cerr << "Error: Could not create directories under " << rootDir << "\n";
        return;
    }

    Project p;
    p.version = 1;
    p.meta.name = projectName;
    p.meta.author = "";
    p.meta.units = unit;

    const fs::path pj = rootDir / PROJECT_FILENAME;
    if (!p.Save(pj.string(), /*pretty*/ true)) {
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
              << "  1) ccad parts add\n"
              << "  2) ccad live\n"
              << "  3) ccad build\n";
}

void App::handlePartsAdd(const std::string& partName, const std::string& partMatName) {
    try {
        const fs::path pj = fs::current_path() / PROJECT_FILENAME;
        if (!fs::exists(pj)) {
            std::cerr << "Error: project.json not found in current directory. "
                         "Run `init` command first or cd into a project.\n";
            return;
        }

        Project p;
        try {
            p.Load(pj.string());
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

        // Add all project parameters
        ostringstream oss;
        oss << "-- Useful library included" << endl;
        oss << "--" << endl;
        oss << "local T = require(\"ccad.util.transform\")" << endl;
        oss << "local B = require(\"ccad.util.box\")" << endl;
        oss << "local S = require(\"ccad.util.sketch\")" << endl << endl;

        oss << "-- Project parameters" << endl;
        oss << "--" << endl;
        int i = 1;
        for (auto it = p.params.begin(); it != p.params.end(); ++it, i++) {
            oss << "local P" << i << " = param(\"" << it->first << "\", ";
            if (it->second.type == ParamValue::Type::Number) {
                oss << it->second.number;
            } else if (it->second.type == ParamValue::Type::Boolean) {
                if (it->second.boolean)
                    oss << "true";
                else
                    oss << "false";
            } else if (it->second.type == ParamValue::Type::String) {
                oss << "\"" << it->second.string << "\"";
            }
            oss << ")" << endl;
        }

        // Add template to see a default shape
        std::ofstream(luaPath) << oss.str() << tpl;

        // Add part in project file
        Part pr{};
        pr.id = "";  // optional
        pr.name = partName;
        pr.source = std::string(PARTS_DIR) + "/" + finalFile;
        pr.material = partMatName;
        pr.transform.translate = {0, 0, 0};
        pr.transform.rotate = {0, 0, 0};
        pr.transform.scale = 1;
        pr.visible = true;
        p.parts.push_back(std::move(pr));

        // Save project
        if (!p.Save(pj.string(), /*pretty*/ true)) {
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
    m_Controller->LoadProject(rootDir);
    m_Controller->BuildProject();
}

void App::handleLive(const std::string& rootDir) {
    m_Controller->LoadProject(rootDir);
    m_Controller->ViewProject();
}

void App::handleBom() {
    fs::path root = fs::current_path();
    m_Controller->LoadProject(root);
    m_Controller->CreateBom();
}

void App::handleDoctor() {
    m_Controller->HealthCheck();
}

void App::handleParamsSet(const std::string& key, const std::string& value) {
    auto pj = fs::current_path() / PROJECT_FILENAME;
    Project p;
    p.Load(pj.string());

    auto lower = value;
    for (auto& c : lower) c = (char)std::tolower(c);
    ParamValue v;
    if (lower == "true" || lower == "false" || lower == "1" || lower == "0") {
        v = ParamValue::FromBool(lower == "true" || lower == "1");
    } else {
        char* end = nullptr;
        double num = std::strtod(value.c_str(), &end);
        if (end && *end == '\0')
            v = ParamValue::FromNumber(num);
        else
            v = ParamValue::FromString(value);
    }
    SetParam(p, key, v);

    // Save project
    if (!p.Save(pj.string(), /*pretty*/ true)) {
        std::cerr << "Error: failed to save " << pj << "\n";
        return;
    } else {
        std::cout << "Set " << key << "\n";
    }
}

void App::handleMaterialSet(const std::string& name, const std::string& color) {
    auto pj = fs::current_path() / PROJECT_FILENAME;
    Project p;
    p.Load(pj.string());

    Material m;
    m.color = color;
    p.materials[name] = m;

    if (!p.Save(pj.string(), /*pretty*/ true)) {
        std::cerr << "Error: failed to save " << pj << "\n";
        return;
    } else {
        std::cout << "Set " << name << "\n";
    }
}

void App::handleLspInit() {
    fs::path root = fs::current_path();
    fs::path luarc = root / ".luarc.json";

    std::vector<std::string> workspaceLib = {"${workspaceFolder}/types", "${workspaceFolder}/lib"};

    std::vector<std::string> installLibs;
    for (const auto& p : utils::DefaultInstallLuaPathsFromExe()) {
        auto dir = fs::path(p).parent_path();
        if (std::find(installLibs.begin(), installLibs.end(), dir.string()) == installLibs.end()) {
            installLibs.push_back(dir.string());
        }
    }

    json j;
    j["$schema"] = "https://raw.githubusercontent.com/sumneko/vscode-lua/master/setting/schema.json";
    j["runtime"]["version"] = "Lua 5.4";
    auto libs = workspaceLib;
    libs.insert(libs.end(), installLibs.begin(), installLibs.end());
    j["workspace"]["library"] = libs;
    j["workspace"]["checkThirdParty"] = false;

    j["diagnostics"]["globals"] = {
        "bbox",         "box",     "center_to",   "center_x",     "center_xy",       "center_xyz", "center_y",
        "center_z",     "chamfer", "chamfer_all", "cone",         "curved_plate_xy", "cylinder",   "deg",
        "difference",   "edges",   "emit",        "extrude",      "fillet",          "fillet_all", "hex_prism",
        "intersection", "lathe",   "mm",          "param",        "pipe_adapter",    "poly_xy",    "profile_xz",
        "rect",         "revolve", "rod",         "rotate_x",     "rotate_y",        "rotate_z",   "save_step",
        "save_stl",     "scale",   "sphere",      "threaded_rod", "translate",       "union",      "wedge",
        "ThreadSpec"};

    if (!utils::WriteTextFile(luarc, j.dump(2) + "\n")) {
        std::cerr << "Failed to write .luarc.json: " << std::endl;
        return;
    }
    std::cout << "Wrote " << luarc << "\n";
    std::cout << "Included library paths:\n";
    for (auto& s : libs) std::cout << "  - " << s << "\n";
}
