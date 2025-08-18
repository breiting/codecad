// clang-format off
#include <glad.h>
#include <GLFW/glfw3.h>
// clang-format on
//

#include <CosmaController.hpp>
#include <core/Camera.hpp>
#include <core/CameraOrbit.hpp>
#include <core/DirectionalLight.hpp>
#include <core/Renderer.hpp>
#include <core/Window.hpp>
#include <material/Color.hpp>
#include <material/FlatShadedMaterial.hpp>
#include <material/Material.hpp>
#include <material/Shader.hpp>
#include <scene/AxesNode.hpp>
#include <scene/InfinityGridNode.hpp>
#include <scene/Scene.hpp>

using namespace std;

const glm::vec3 SUN_LIGHT = {1.0f, 0.95f, 0.9f};
const int STATUSBAR_TIMEOUT_MS = 3000;

CosmaController::CosmaController()
    : m_LeftMouseButtonPressed(false), m_RightMouseButtonPressed(false), m_ShiftPressed(false) {
}

void CosmaController::Init(Window* window) {
    // Setup camera
    m_Camera = std::make_unique<CameraOrbit>(50.0);
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

void CosmaController::LoadProject(const Project& project) {
    // m_FileWatcher = std::make_unique<FileWatcher>(fileName, std::chrono::milliseconds(300));
    // SetStatusMessage(fileName + " (manifest) loaded");
}

void CosmaController::Update(float deltaTime) {
    m_Camera->Update(deltaTime);

    // Single-STL Live-Reload
    if (m_FileWatcher) {
        m_FileWatcher->Poll([&](const std::string& fileName) { SetStatusMessage("File changed!"); });
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
