#include "Controller.hpp"

#include <filesystem>
#include <iostream>

#include "io/Bom.hpp"
#include "io/Export.hpp"
#include "io/Paths.hpp"

using namespace std;

const std::string PROJECT_FILENAME = "project.json";
const std::string PROJECT_OUTDIR = "generated";

static void ApplyProjectParamsToLua(sol::state& L, const io::Project& p) {
    sol::table P = L["PARAMS"];
    if (!P.valid()) P = L.create_named_table("PARAMS");

    for (const auto& kv : p.params) {
        const auto& k = kv.first;
        const auto& v = kv.second;
        switch (v.type) {
            case io::ParamValue::Type::Boolean:
                P[k] = v.boolean;
                break;
            case io::ParamValue::Type::Number:
                P[k] = v.number;
                break;
            case io::ParamValue::Type::String:
                P[k] = v.string;
                break;
        }
    }
}

Controller::Controller(std::vector<std::string>& luaPaths) : m_LuaPaths(luaPaths) {
}

void Controller::LoadProject(const fs::path& projectDir) {
    m_ProjectDir = projectDir;

    const fs::path pj = projectDir / PROJECT_FILENAME;
    if (!fs::exists(pj)) {
        throw std::runtime_error(
            std::string(PROJECT_FILENAME + " not found in directory. Run `init` command first or cd into a project."));
    }

    try {
        m_Project = io::LoadProject(pj.string());
    } catch (const std::exception& e) {
        std::cerr << "Failed to load project: " << e.what() << "\n";
        return;
    }

    SetupEngine();
    ApplyProjectParamsToLua(m_Engine->Lua(), m_Project);

    m_ProjectLoaded = true;

    //  PURE Controller
    // pure = std::make_unique<pure::PureController>();
    // if (!pure->Initialize(1280, 800, "CodeCAD Viewer", err)) return false;

    // 4) Bridge
    // bridge = std::make_unique<RenderBridge>(pure->Scene());

    // 5) Initiale Szene
    // std::string buildErr;
    // if (!bridge->BuildProject(engine, project, &buildErr)) {
    //     if (err) *err = buildErr;
    //     return false;
    // }

    // 6) Watcher setzen
    // projectWatch = FileWatcher(pj);
    // for (const auto& part : project.parts) {
    //     std::string lp = (fs::path(rootDir) / part.source).string();
    //     luaWatchers.emplace(lp, FileWatcher(lp));
    // }
}
void Controller::BuildProject() {
    if (!m_ProjectLoaded) {
        throw std::runtime_error("No project is loaded!");
    }

    io::PrintProject(m_Project);
    auto projectRoot = std::filesystem::absolute(std::filesystem::path(m_ProjectDir));

    auto outDir = projectRoot / PROJECT_OUTDIR;
    fs::create_directory(outDir);

    for (const auto& jp : m_Project.parts) {
        std::filesystem::path src = projectRoot / jp.source;
        auto luaFile = std::filesystem::weakly_canonical(src);

        std::string err;
        ApplyProjectParamsToLua(m_Engine->Lua(), m_Project);

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
void Controller::ViewProject() {
    if (!m_ProjectLoaded) {
        throw std::runtime_error("No project is loaded!");
    }
    throw std::runtime_error("not implemented!");
}

void Controller::CreateBom() {
    if (!m_ProjectLoaded) {
        throw std::runtime_error("No project is loaded!");
    }
    try {
        io::BomWriter bomWriter;

        for (const auto& part : m_Project.parts) {
            fs::path src = fs::current_path() / part.source;
            fs::path luaFile = fs::weakly_canonical(src);

            try {
                sol::state& L = m_Engine->Lua();
                sol::table bom = L["require"]("ccad.util.bom");
                if (bom.valid()) {
                    sol::protected_function clear = bom["clear"];
                    if (clear.valid()) {
                        sol::protected_function_result r = clear();
                        if (!r.valid()) {
                            sol::error err = r;
                            std::cerr << "Warning: bom.clear() failed: " << err.what() << "\n";
                        }
                    }
                }
            } catch (...) {
                std::cerr << "Warning: could not clear BOM for part\n";
            }

            std::string err;
            ApplyProjectParamsToLua(m_Engine->Lua(), m_Project);
            if (!m_Engine->RunFile(luaFile.string(), &err)) {
                std::cerr << "Lua error in " << luaFile.string() << ": " << err << "\n";
                return;
            }

            try {
                bomWriter.Collect(m_Engine->Lua(), part.id.empty() ? part.name : part.id);
            } catch (const std::exception& e) {
                std::cerr << "Error: collecting BOM failed for part " << (part.id.empty() ? part.name : part.id) << ": "
                          << e.what() << "\n";
                return;
            }
        }

        bomWriter.WriteCsv("bom.csv");
        bomWriter.WriteMarkdown("bom.md");
        std::cout << "BOM written: bom.csv, bom.md\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: failed to generate BOM: " << e.what() << "\n";
    }
}

void Controller::SetupEngine() {
    m_Engine = std::make_shared<LuaEngine>();

    // Standard search paths
    std::vector<std::string> paths = {"./lib/?.lua", "./lib/?/init.lua", "./vendor/?.lua", "./vendor/?/init.lua"};

    // installed standard paths
    auto installPaths = io::DefaultInstallLuaPathsFromExe();
    paths.insert(paths.end(), installPaths.begin(), installPaths.end());

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

// void Controller::OnProjectChanged() {
//     try {
//         project = io::LoadProject(rootDir + "/project.json");
//         engine.SetParams(project.params);
//         // Watcher neu setzen, evtl. neue Teile
//         luaWatchers.clear();
//         for (const auto& part : project.parts) {
//             std::string lp = (fs::path(rootDir) / part.source).string();
//             luaWatchers.emplace(lp, FileWatcher(lp));
//         }
//         // komplette Szene neu aufbauen
//         std::string err;
//         bridge->BuildProject(engine, project, &err);
//         if (!err.empty()) std::cerr << err << "\n";
//         pure->SetStatus("Project reloaded");
//     } catch (const std::exception& e) {
//         std::cerr << "Reload failed: " << e.what() << "\n";
//     }
// }
//
// void Controller::OnLuaChanged(const std::string& luaPath) {
//     // finde Part
//     for (const auto& part : project.parts) {
//         auto src = (fs::path(rootDir) / part.source).string();
//         if (src == luaPath) {
//             PartVisual pv;
//             std::string err;
//             if (bridge->BuildPart(engine, part, &pv, &err)) {
//                 bridge->UpsertPartNode(pv);
//                 pure->SetStatus(("Rebuilt " + part.id).c_str());
//             } else {
//                 pure->SetStatus(("Error: " + err).c_str());
//             }
//             return;
//         }
//     }
// }
//
// void Controller::Run() {
//     while (!pure->ShouldClose()) {
//         // Live-Reload polling (leichtes Debounce in deinem FileWatcher)
//         projectWatch.Poll([&](const std::string&) { OnProjectChanged(); });
//         for (auto& kv : luaWatchers) {
//             kv.second.Poll([&](const std::string& path) { OnLuaChanged(path); });
//         }
//         pure->RunFrame();  // rendert Scene + GUI + Axis etc.
//     }
// }
//
void Controller::HealthCheck() {
    cout << "== CodeCAD Doctor ==\n";
    auto exe = io::ExecutablePath();
    cout << "Executable: " << (exe.empty() ? "<unknown>" : exe.string()) << "\n";

    auto installPatterns = io::DefaultInstallLuaPathsFromExe();
    cout << "Derived install search patterns:\n";
    for (auto& p : installPatterns) cout << "  " << p << "\n";

    auto& L = m_Engine->Lua();
    try {
        std::string pkg_path = L["package"]["path"];
        cout << "\nLua package.path:\n" << pkg_path << "\n";
    } catch (...) { /* ignore */
    }

    // Minimal-Checks
    auto try_require = [&](const char* mod) {
        try {
            sol::load_result lr = m_Engine->Lua().load(("return require('" + std::string(mod) + "')").c_str());
            if (!lr.valid()) {
                sol::error e = lr;
                cout << "  " << mod << ": load error: " << e.what() << "\n";
                return;
            }
            sol::protected_function_result r = lr();
            if (!r.valid()) {
                sol::error e = r;
                cout << "  " << mod << ": runtime error: " << e.what() << "\n";
                return;
            }
            cout << "  " << mod << ": OK\n";
        } catch (const std::exception& e) {
            cout << "  " << mod << ": exception: " << e.what() << "\n";
        }
    };

    cout << "\nRequire checks:\n";
    try_require("ccad.util.box");
    try_require("ccad.util.transform");
    try_require("ccad.util.func");
    try_require("ccad.util.place");
    try_require("ccad.util.shape2d");
    try_require("ccad.util.sketch");

    cout << "\nEnvironment:\n";
#ifdef __APPLE__
    cout << "  OS: macOS\n";
#elif __linux__
    cout << "  OS: Linux\n";
#else
    cout << "  OS: (unknown)\n";
#endif

    try {
        auto lr = L.load(R"(
      local ok = pcall(function() return box(1,1,1) end)
      return ok
    )");
        auto r = lr();
        bool ok = r.get<bool>();
        cout << "  Primitive test (box): " << (ok ? "OK" : "FAILED") << "\n";
    } catch (...) {
        cout << "  Primitive test (box): FAILED\n";
    }

    cout << "\nGood to know:\n";
    cout << "  - In case of neovim completion problems: 'ccad lsp' should fix it\n";
    cout << "  - Make sure that all paths are set properly\n";
}
