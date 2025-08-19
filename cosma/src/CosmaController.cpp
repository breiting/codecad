// clang-format off
#include <glad.h>
#include <GLFW/glfw3.h>
// clang-format on
//

#include <BRepBuilderAPI_Transform.hxx>
#include <CosmaController.hpp>
#include <core/Camera.hpp>
#include <core/CameraOrbit.hpp>
#include <core/DirectionalLight.hpp>
#include <core/Renderer.hpp>
#include <core/Window.hpp>
#include <gp_Ax1.hxx>
#include <gp_Quaternion.hxx>
#include <gp_Trsf.hxx>
#include <material/Color.hpp>
#include <material/FlatShadedMaterial.hpp>
#include <material/Material.hpp>
#include <material/Shader.hpp>
#include <scene/AxesNode.hpp>
#include <scene/InfinityGridNode.hpp>
#include <scene/Scene.hpp>

#include "core/FileWatcher.hpp"
#include "geometry/Mesh.hpp"
#include "io/Project.hpp"

using namespace std;

const glm::vec3 SUN_LIGHT = {1.0f, 0.95f, 0.9f};
const int STATUSBAR_TIMEOUT_MS = 3000;

CosmaController::CosmaController(std::unique_ptr<CoreEngine> coreEngine)
    : m_Engine(std::move(coreEngine)),
      m_LeftMouseButtonPressed(false),
      m_RightMouseButtonPressed(false),
      m_ShiftPressed(false) {
}

void CosmaController::Init(Window* window) {
    // Setup camera
    m_Camera = std::make_unique<CameraOrbit>(400.0);
    m_Camera->SetAspectRatio(static_cast<float>(window->GetWidth()) / static_cast<float>(window->GetHeight()));

    // Create new scene
    m_Scene = std::make_unique<Scene>();

    auto axesNode = std::make_shared<AxesNode>();
    m_Scene->AddNode(axesNode);
    //
    auto gridNode = std::make_shared<InfinityGridNode>();
    // m_Scene->AddNode(gridNode);

    // Create new light
    auto light = std::make_shared<DirectionalLight>();
    light->SetColor(SUN_LIGHT);
    m_Scene->SetLightDirectionalLight(light);

    // Setup renderer
    m_Renderer = std::make_unique<Renderer>();

    // Setup window
    m_Window = window;

    // Setup GUI
    m_Gui = std::make_unique<GuiLayer>();
    m_Gui->Init(window);
}

geometry::ShapePtr CosmaController::ApplyTransform(const geometry::ShapePtr& s, const io::Transform& tr) {
    if (!s || s->Get().IsNull()) return s;

    gp_Trsf t;

    // Scale
    t.SetScale(gp_Pnt(0, 0, 0), tr.scale);

    // Rotation (deg) – ZYX Reihenfolge (konventionell)
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

static glm::vec3 ParseHexColor(const std::string& hex, glm::vec3 fallback = {0.7f, 0.7f, 0.7f}) {
    if (hex.size() != 7 || hex[0] != '#') return fallback;
    auto to01 = [](int v) { return static_cast<float>(v) / 255.0f; };
    int r = std::stoi(hex.substr(1, 2), nullptr, 16);
    int g = std::stoi(hex.substr(3, 2), nullptr, 16);
    int b = std::stoi(hex.substr(5, 2), nullptr, 16);
    return {to01(r), to01(g), to01(b)};
}

std::shared_ptr<MeshNode> CosmaController::BuildMeshNodeFromShape(const TopoDS_Shape& s, const std::string& colorHex) {
    geometry::TriMesh tri = geometry::TriangulateShape(s, /*defl*/ 0.3, /*ang*/ 25.0, /*parallel*/ true);

    auto mesh = std::make_unique<Mesh>();

    for (const auto& p : tri.positions) mesh->AddVertex(p);
    for (unsigned idx : tri.indices) mesh->AddIndex(idx);
    mesh->RecalculateNormals();

    auto node = std::make_shared<MeshNode>(std::move(mesh));
    auto mat = std::make_shared<FlatShadedMaterial>();
    mat->SetMaterialColor(ParseHexColor(colorHex));
    node->SetMaterial(mat);
    return node;
}

void CosmaController::BuildOrRebuildPart(PartRecord& rec) {
    m_Engine->Reset();

    std::string err;
    if (!m_Engine->RunFile(rec.absoluteSourcePath.string(), &err)) {
        auto errStr = std::string("Lua error in ") + rec.meta.name + ": " + err;
        cerr << errStr << endl;
        SetStatusMessage(errStr);
        return;
    }

    auto emitted = m_Engine->GetEmitted();
    if (!emitted) {
        SetStatusMessage(std::string("No emit() in ") + rec.meta.name);
        return;
    }

    // Apply transform from project.json
    auto shaped = ApplyTransform(*emitted, rec.meta.transform);

    // MeshNode generation
    auto color = m_Project.materials[rec.meta.material].color;
    auto newNode = BuildMeshNodeFromShape(shaped->Get(), color.empty() ? "#cccccc" : color);

    if (rec.node) {
        m_Scene->RemoveNode(rec.node);
        rec.node = newNode;
        m_Scene->AddNode(rec.node);
    } else {
        rec.node = newNode;
        m_Scene->AddNode(rec.node);
    }

    rec.shape = shaped;  // optional behalten
}

void CosmaController::LoadLuaPartByPath(const std::string& path) {
    auto itName = m_SourceToPart.find(path);
    if (itName == m_SourceToPart.end()) return;

    auto itRec = m_PartsByName.find(itName->second);
    if (itRec == m_PartsByName.end()) return;

    try {
        BuildOrRebuildPart(itRec->second);
        SetStatusMessage("Rebuilt part: " + itRec->second.meta.name);
    } catch (const std::exception& e) {
        SetStatusMessage(std::string("Rebuild failed: ") + itRec->second.meta.name + " : " + e.what());
    }
}

void CosmaController::LoadProject(const std::string& projectFileName) {
    // Project
    m_Project = io::LoadProject(projectFileName);
    io::PrintProject(m_Project);

    m_ProjectRoot = std::filesystem::absolute(std::filesystem::path(projectFileName)).parent_path();

    m_FileWatcher[PROJECT_KEY] = FileWatcher(projectFileName);

    for (auto it = m_PartsByName.begin(); it != m_PartsByName.end(); ++it) {
        m_Scene->RemoveNode(it->second.node);
    }

    m_PartsByName.clear();
    m_SourceToPart.clear();

    for (const auto& jp : m_Project.parts) {
        PartRecord rec;
        rec.meta = jp;

        // Normalize source path
        std::filesystem::path src = jp.source.empty() ? "" : (m_ProjectRoot / jp.source);
        rec.absoluteSourcePath = std::filesystem::weakly_canonical(src);

        // Watcher je Lua
        if (!rec.absoluteSourcePath.empty()) {
            m_FileWatcher[rec.absoluteSourcePath.string()] = FileWatcher(rec.absoluteSourcePath.string());
            m_SourceToPart[rec.absoluteSourcePath.string()] = jp.name;
        }

        if (jp.source.size() && (jp.visible)) {
            try {
                BuildOrRebuildPart(rec);
            } catch (const std::exception& e) {
                SetStatusMessage(std::string("Build part failed: ") + jp.name + " : " + e.what());
            }
        }

        m_PartsByName[jp.name] = std::move(rec);
    }

    SetStatusMessage(projectFileName + " loaded");
}

void CosmaController::Update(float deltaTime) {
    m_Camera->Update(deltaTime);

    for (auto& kv : m_FileWatcher) {
        const std::string& key = kv.first;
        auto& watcher = kv.second;

        watcher.Poll([this, key](const std::string& changedPath) {
            if (key == PROJECT_KEY) {
                SetStatusMessage("Project changed, reloading ...");
                this->LoadProject(changedPath);
                return;
            }
            this->LoadLuaPartByPath(key);
        });
    }

    // update headlight to camera position
    m_Scene->GetDirectionalLight()->SetDirection(m_Camera->GetViewDirection());
    m_Scene->Update(deltaTime);
}

void CosmaController::Render() {
    m_Renderer->Render(m_Camera.get(), m_Scene.get(), m_Gui.get());
    DrawGui();
    m_Gui->End();
}

void CosmaController::SetStatusMessage(const std::string& msg) {
    m_StatusMessage = msg;
    m_StatusTimestamp = std::chrono::steady_clock::now();
}

void CosmaController::DrawGui() {
    ImGuiIO& io = ImGui::GetIO();
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    float barHeight = ImGui::GetFrameHeight() + 4.0f;
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - barHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, barHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground;

    ImGui::Begin("StatusBar", nullptr, flags);

    // --- Left: Status messages with timout ---
    auto now = std::chrono::steady_clock::now();
    bool showStatus =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - m_StatusTimestamp).count() < STATUSBAR_TIMEOUT_MS;

    if (showStatus && !m_StatusMessage.empty()) {
        ImGui::TextUnformatted(m_StatusMessage.c_str());
    }

    // --- Right: FPS ---
    float fps = io.Framerate;
    std::string fpsText = std::string("FPS: ") + std::to_string((int)fps);

    float textWidth = ImGui::CalcTextSize(fpsText.c_str()).x;
    ImGui::SameLine(viewport->Size.x - textWidth - 10.0f);  // right aligned
    ImGui::TextUnformatted(fpsText.c_str());

    ImGui::End();
}

void CosmaController::Shutdown() {
    m_Gui->Shutdown();
}

void CosmaController::OnFramebufferSize(int width, int height) {
    m_Renderer->Resize(width, height);
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    m_Camera->SetAspectRatio(aspect);
}

void CosmaController::OnMouseMove(double xpos, double ypos) {
    if (m_LeftMouseButtonPressed) {
        m_Camera->OnMouseRotation(xpos, ypos);
    } else if (m_RightMouseButtonPressed) {
        m_Camera->OnMousePan(xpos, ypos);
    }
}

void CosmaController::OnScroll(double xoffset, double yoffset) {
    m_Camera->OnMouseScroll(yoffset);
}

void CosmaController::OnKeyPressed(int key, int mods) {
    if (key == GLFW_KEY_ESCAPE) {
        m_Window->Close();
    } else if (key == GLFW_KEY_W) {
        m_Renderer->ToggleWireframe();
    } else if (key == GLFW_KEY_LEFT_BRACKET) {
        m_TargetT = 0.0f;
    } else if (key == GLFW_KEY_RIGHT_BRACKET) {
        m_TargetT = 1.0f;
    }

    m_ShiftPressed = (mods & GLFW_MOD_SHIFT) != 0;
}

void CosmaController::OnKeyReleased(int key, int mods) {
    m_ShiftPressed = (mods & GLFW_MOD_SHIFT) != 0;
}

void CosmaController::OnMouseButtonPressed(int button, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        m_LeftMouseButtonPressed = true;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        m_RightMouseButtonPressed = true;
    }
    m_Camera->OnMouseStart();
}

void CosmaController::OnMouseButtonReleased(int button, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        m_LeftMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        m_RightMouseButtonPressed = false;
    }
}
