#include "Controller.hpp"

#include <ccad/base/Logger.hpp>
#include <ccad/io/Export.hpp>
#include <ccad/lua/Bom.hpp>
#include <ccad/lua/LuaEngine.hpp>
#include <ccad/ops/Transform.hpp>
#include <exception>
#include <filesystem>
#include <iostream>

#include "GLFW/glfw3.h"
#include "ProjectPanel.hpp"
#include "Utils.hpp"
#include "pure/PureBounds.hpp"
#include "pure/PureMeasurement.hpp"
#include "pure/PureMesh.hpp"
#include "pure/PurePicker.hpp"
#include "pure/PureTypes.hpp"

using namespace std;
using namespace pure;
namespace fs = std::filesystem;

const std::string PROJECT_FILENAME = "project.json";
const std::string PROJECT_OUTDIR = "generated";

std::string Controller::NormalizePath(const std::string& p) {
    try {
        return fs::weakly_canonical(p).string();
    } catch (...) {
        return p;
    }
}

static glm::vec3 ParseHexColor(const std::string& hex, glm::vec3 fallback = {0.7f, 0.7f, 0.7f}) {
    if (hex.size() != 7 || hex[0] != '#') return fallback;
    auto to01 = [](int v) { return static_cast<float>(v) / 255.0f; };
    int r = std::stoi(hex.substr(1, 2), nullptr, 16);
    int g = std::stoi(hex.substr(3, 2), nullptr, 16);
    int b = std::stoi(hex.substr(5, 2), nullptr, 16);
    return {to01(r), to01(g), to01(b)};
}

static void ApplyProjectParamsToLua(sol::state& L, const Project& p) {
    sol::table P = L["PARAMS"];
    if (!P.valid()) P = L.create_named_table("PARAMS");

    for (const auto& kv : p.params) {
        const auto& k = kv.first;
        const auto& v = kv.second;
        switch (v.type) {
            case ParamValue::Type::Boolean:
                P[k] = v.boolean;
                break;
            case ParamValue::Type::Number:
                P[k] = v.number;
                break;
            case ParamValue::Type::String:
                P[k] = v.string;
                break;
        }
    }
}

ccad::Shape ApplyProjectTransform(const ccad::Shape& s, const PartTransform& tr) {
    if (!s) return s;

    const bool noScale = std::abs(tr.scale - 1.0) < 1e-12;
    const bool noRot = tr.rotate.x == 0.0 && tr.rotate.y == 0.0 && tr.rotate.z == 0.0;
    const bool noTrans = tr.translate.x == 0.0 && tr.translate.y == 0.0 && tr.translate.z == 0.0;
    if (noScale && noRot && noTrans) return s;

    ccad::Shape out = s;
    // gleiche Reihenfolge wie vorher: S, Rx, Ry, Rz, T
    if (!noScale) out = ccad::ops::ScaleUniform(out, tr.scale);
    if (tr.rotate.x != 0.0) out = ccad::ops::RotateX(out, tr.rotate.x);
    if (tr.rotate.y != 0.0) out = ccad::ops::RotateY(out, tr.rotate.y);
    if (tr.rotate.z != 0.0) out = ccad::ops::RotateZ(out, tr.rotate.z);
    if (!noTrans) out = ccad::ops::Translate(out, tr.translate.x, tr.translate.y, tr.translate.z);
    return out;
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
        m_Project.Load(pj.string());
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

    m_Project.Print();
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
        ccad::io::SaveSTL(emitted.value(), stlFile.string(), ccad::lua::GetTriangulationParameters());
    }
}
void Controller::ViewProject() {
    if (!m_ProjectLoaded) {
        throw std::runtime_error("No project is loaded!");
    }

    if (!m_PureController.Initialize(1600, 1200, "CodeCAD Viewer", utils::DefaultInstallFontsPath())) {
        std::cerr << "Failed to initialize CodeCAD Viewer\n";
        return;
    }

    ProjectPanel panel(m_Project);
    panel.SetOnSave([this](const Project& p) { p.Save(fs::path(m_ProjectDir) / PROJECT_FILENAME, /*pretty*/ true); });

    m_PureController.SetRightDockPanel([&panel]() { panel.Draw(); });

    m_PureController.SetMouseMoveHandler([this](double x, double y) { m_Picker->UpdateHover(x, y); });
    m_PureController.SetMouseButtonHandler([this](int /*button*/, int action, int /*mods*/) {
        bool pressed = (action == GLFW_PRESS);
        if (pressed) {
            auto hit = m_Picker->GetHoverState();
            if (hit.kind == PurePicker::HoverKind::None) return;

            if (hit.kind == PurePicker::HoverKind::Vertex) {
                m_Measure.OnPick(PickPoint{hit.pos});
            } else if (hit.kind == PurePicker::HoverKind::Edge) {
                if (hit.edge.has_value()) {
                    m_Measure.OnPick(PickEdge{hit.edge->a, hit.edge->b});
                }
            }
        }
    });

    m_PureController.SetKeyPressedHandler([this](int key, int /*mods*/) {
        switch (key) {
            case GLFW_KEY_W: {
                m_PureController.ToggleWireframe();
            } break;
            case GLFW_KEY_S: {
                BuildProject();
            } break;
            case GLFW_KEY_M: {
                m_AppMode = AppMode::Measure;
                m_Measure.Enable(true);
            } break;
            case GLFW_KEY_O: {
                m_AppMode = AppMode::Orbit;
                m_Measure.Enable(false);
            } break;

            default:
                break;
        }
    });

    m_Scene = std::make_shared<PureScene>();

    // Initialize Picker & Measure
    m_Picker = std::make_unique<PurePicker>();
    m_Picker->SetSnapPixels(8.0f);  // 8px Snapradius
    m_Picker->SetScene(m_Scene.get());
    m_Measure.SetReporter([this](const std::string& s) { this->m_PureController.SetStatus(s); });

    RebuildAllParts();
    SetupWatchers();

    // Set camera only upon first load
    PureBounds bounds;
    if (m_Scene->ComputeBounds(bounds)) {
        m_PureController.Camera()->FitToBounds(bounds, 1.12f);
    }

    while (!m_PureController.ShouldClose()) {
        PollWatchers();

        m_PureController.BeginFrame();

        m_PureController.DrawGui();
        m_PureController.RenderScene(m_Scene);

        if (m_AppMode == AppMode::Measure) {
            glm::mat4 view = m_PureController.Camera()->View();
            glm::mat4 proj = m_PureController.Camera()->Projection();

            m_Picker->SetViewProj(view, proj);
            m_Picker->SetViewport(m_PureController.GetFramebufferWidth(), m_PureController.GetFramebufferHeight());
            ImDrawList* fg = ImGui::GetForegroundDrawList();
            m_Picker->DrawHoverOverlay(fg);
        }

        m_PureController.EndFrame();
    }

    m_PureController.Shutdown();
}

void Controller::SetupWatchers() {
    // Project watcher
    m_ProjectWatcher = FileWatcher(fs::path(m_ProjectDir) / PROJECT_FILENAME);

    // Lua watchers
    ResetLuaWatchers();
}

void Controller::ResetLuaWatchers() {
    m_LuaWatchers.clear();
    m_LuaToPartId.clear();
    m_LastLuaEvent.clear();

    for (const auto& part : m_Project.parts) {
        fs::path src = fs::path(m_ProjectDir) / part.source;
        const std::string abs = NormalizePath(src.string());
        m_LuaWatchers.emplace(abs, FileWatcher(abs));
        m_LuaToPartId[abs] = part.id;
    }
}

void Controller::PollWatchers() {
    // project.json
    m_ProjectWatcher.Poll([this](const std::string&) {
        auto now = std::chrono::steady_clock::now();
        if (m_LastProjectEvent.time_since_epoch().count() != 0) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LastProjectEvent).count();
            if (ms < m_DebounceMs) return;
        }
        m_LastProjectEvent = now;
        OnProjectChanged();
    });

    // Part .lua files
    for (auto& kv : m_LuaWatchers) {
        const std::string path = kv.first;
        FileWatcher& fw = kv.second;

        fw.Poll([this, path](const std::string&) {
            auto now = std::chrono::steady_clock::now();
            auto it = m_LastLuaEvent.find(path);
            if (it != m_LastLuaEvent.end()) {
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count();
                if (ms < m_DebounceMs) return;
            }
            m_LastLuaEvent[path] = now;
            OnLuaChanged(path);
        });
    }
}

void Controller::ClearScene() {
    if (m_Scene) m_Scene->Clear();
}

void Controller::AddPartToScene(const Part& part) {
    fs::path src = fs::path(m_ProjectDir) / part.source;
    auto luaFile = std::filesystem::weakly_canonical(src);

    std::optional<ccad::Shape> emitted;
    try {
        std::string err;
        if (!m_Engine->RunFile(luaFile, &err)) {
            auto errStr = std::string("Lua error in ") + luaFile.string() + ": " + err;
            std::cerr << errStr << std::endl;
            return;
        }

        emitted = m_Engine->GetEmitted();
        if (!emitted) {
            std::cerr << "Canot get shape from " << luaFile << std::endl;
            return;
        }
    } catch (std::exception& e) {
        LOG(ERROR) << "Error during processing lua file: " << e.what();
        return;
    }

    // Apply transform from project.json
    auto shaped = ApplyProjectTransform(*emitted, part.transform);

    auto color = m_Project.materials[part.material].color;
    if (color.empty()) color = "#cccccc";

    ccad::geom::TriMesh tri = ccad::geom::Triangulate(shaped, ccad::lua::GetTriangulationParameters());

    std::vector<PureVertex> vertices;

    for (const auto& v : tri.positions) {
        vertices.push_back({glm::vec3(v.x, v.y, v.z), glm::vec3(0.0, 0.0, 0.0)});  // no normals!
    }

    auto mesh = std::make_shared<PureMesh>();
    mesh->Upload(vertices, tri.indices);
    m_Scene->AddPart(part.id, mesh, glm::mat4(1.0f), ParseHexColor(color));
}

void Controller::RebuildAllParts() {
    ClearScene();
    for (const auto& part : m_Project.parts) {
        if (part.visible) {
            AddPartToScene(part);
        }
    }
}

void Controller::RebuildPartByPath(const std::string& luaPath) {
    // Part finden
    auto it = m_LuaToPartId.find(luaPath);
    if (it == m_LuaToPartId.end()) {
        // Part not known? Then reload everything (fallback)
        RebuildAllParts();
        return;
    }
    const std::string& partId = it->second;

    m_Scene->RemovePartById(partId);
    const Part* p = nullptr;
    for (const auto& pr : m_Project.parts)
        if (pr.id == partId) {
            p = &pr;
            break;
        }
    if (!p) return;
    AddPartToScene(*p);
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
    m_Engine = std::make_shared<ccad::lua::LuaEngine>();

    // Standard search paths
    std::vector<std::string> paths = {"./lib/?.lua", "./lib/?/init.lua", "./vendor/?.lua", "./vendor/?/init.lua"};

    // installed standard paths
    auto installPaths = utils::DefaultInstallLuaPathsFromExe();
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

void Controller::OnProjectChanged() {
    try {
        m_PureController.SetStatus("Project changed. Reloading...");
        m_Scene->Clear();
        m_Project.Load(fs::path(m_ProjectDir) / PROJECT_FILENAME);

        ApplyProjectParamsToLua(m_Engine->Lua(), m_Project);
        ResetLuaWatchers();
        RebuildAllParts();

        m_PureController.SetStatus("Project reloaded.");
    } catch (const std::exception& e) {
        std::cerr << "Project reload failed: " << e.what() << "\n";
        m_PureController.SetStatus("Project reload failed.");
    }
}

void Controller::OnLuaChanged(const std::string& luaPath) {
    try {
        // only rebuild affected part
        RebuildPartByPath(luaPath);
        m_PureController.SetStatus("Part rebuilt.");
    } catch (const std::exception& e) {
        std::cerr << "Part rebuild failed: " << e.what() << " (" << luaPath << ")\n";
        m_PureController.SetStatus("Part rebuild failed.");
    }
}

void Controller::HealthCheck() {
    cout << "== CodeCAD Doctor ==\n";
    auto exe = utils::ExecutablePath();
    cout << "Executable: " << (exe.empty() ? "<unknown>" : exe.string()) << "\n";

    auto installPatterns = utils::DefaultInstallLuaPathsFromExe();
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
