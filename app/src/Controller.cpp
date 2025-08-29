#include "Controller.hpp"

#include <BRepBuilderAPI_Transform.hxx>
#include <filesystem>
#include <gp_Ax1.hxx>
#include <gp_Quaternion.hxx>
#include <gp_Trsf.hxx>
#include <iostream>

#include "io/Bom.hpp"
#include "io/Export.hpp"
#include "io/Paths.hpp"
#include "pure/PureMesh.hpp"

using namespace std;
using namespace pure;

const std::string PROJECT_FILENAME = "project.json";
const std::string PROJECT_OUTDIR = "generated";

static glm::vec3 ParseHexColor(const std::string& hex, glm::vec3 fallback = {0.7f, 0.7f, 0.7f}) {
    if (hex.size() != 7 || hex[0] != '#') return fallback;
    auto to01 = [](int v) { return static_cast<float>(v) / 255.0f; };
    int r = std::stoi(hex.substr(1, 2), nullptr, 16);
    int g = std::stoi(hex.substr(3, 2), nullptr, 16);
    int b = std::stoi(hex.substr(5, 2), nullptr, 16);
    return {to01(r), to01(g), to01(b)};
}

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

geometry::ShapePtr ApplyProjectTransform(const geometry::ShapePtr& s, const io::Transform& tr) {
    if (!s || s->Get().IsNull()) return s;

    gp_Trsf t;

    // Scale
    t.SetScale(gp_Pnt(0, 0, 0), tr.scale);

    // Rotation (deg) – ZYX sequence
    auto deg2rad = [](double d) { return d * M_PI / 180.0; };
    gp_Trsf rx, ry, rz;
    if (tr.rotate.x != 0) {
        rx.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0)), deg2rad(tr.rotate.x));
        t = rx * t;
    }
    if (tr.rotate.y != 0) {
        ry.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0)), deg2rad(tr.rotate.y));
        t = ry * t;
    }
    if (tr.rotate.z != 0) {
        rz.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), deg2rad(tr.rotate.z));
        t = rz * t;
    }

    // Translation
    if (tr.translate.x != 0 || tr.translate.y != 0 || tr.translate.z != 0) {
        gp_Trsf tt;
        tt.SetTranslation(gp_Vec(tr.translate.x, tr.translate.y, tr.translate.z));
        t = tt * t;
    }

    TopoDS_Shape res = BRepBuilderAPI_Transform(s->Get(), t, /*copy*/ true).Shape();
    return std::make_shared<geometry::Shape>(res);
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

    PureController pureController;
    if (!pureController.Initialize(1280, 800, "CodeCAD Viewer")) {
        std::cerr << "Failed to initialize CodeCAD Viewer\n";
        return;
    }

    // Register for mouse and keyboard events
    pureController.SetInputHandler(this);

    auto scene = std::make_shared<PureScene>();

    for (const auto& part : m_Project.parts) {
        fs::path src = fs::path(m_ProjectDir) / part.source;
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

        // Apply transform from project.json
        auto shaped = ApplyProjectTransform(*emitted, part.transform);

        auto color = m_Project.materials[part.material].color;
        if (color.empty()) color = "#cccccc";

        geometry::TriMesh tri =
            geometry::TriangulateShape(shaped->Get(), /*defl*/ 0.3, /*ang*/ 25.0, /*parallel*/ true);

        std::vector<PureVertex> vertices;

        for (const auto& v : tri.positions) {
            vertices.push_back({glm::vec3(v.x, v.y, v.z), glm::vec3(0.0, 0.0, 0.0)});  // no normals!
        }

        auto mesh = std::make_shared<PureMesh>();
        mesh->Upload(vertices, tri.indices);
        scene->AddPart(mesh, glm::mat4(1.0f), ParseHexColor(color));
    }

    // 6) Watcher setzen
    // projectWatch = FileWatcher(pj);
    // for (const auto& part : project.parts) {
    //     std::string lp = (fs::path(rootDir) / part.source).string();
    //     luaWatchers.emplace(lp, FileWatcher(lp));
    // }

    pureController.Run(scene);
    pureController.Shutdown();
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

void Controller::OnKeyPressed(int key, int /*mods*/) {
    std::cout << "Key pressed " << key << std::endl;
}

void Controller::OnKeyReleased(int key, int /*mods*/) {
    std::cout << "Key released " << key << std::endl;
}

void Controller::OnMouseButtonPressed(int button, int /*mods*/) {
    std::cout << "Mouse BT pressed " << button << std::endl;
}

void Controller::OnMouseButtonReleased(int button, int /*mods*/) {
    std::cout << "Mouse BT released " << button << std::endl;
}
