#include <imgui.h>
#include <stb_image_write.h>

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <pure/PureController.hpp>
#include <pure/PureMeshFactory.hpp>
#include <pure/PureRenderArea.hpp>
#include <pure/PureRenderer.hpp>

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "pure/IPureCamera.hpp"
#include "pure/PureBounds.hpp"
#include "pure/PureShader.hpp"

using namespace std;

namespace pure {

const int STATUSBAR_TIMEOUT_MS = 3000;
const float SIDEBAR_WIDTH = 360;

static void ErrorCallback(int code, const char* msg) {
    std::cerr << "GLFW error " << code << ": " << msg << "\n";
}

PureController::PureController() {
}

PureController::~PureController() {
    if (m_Window) {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }
}

bool PureController::Initialize(int width, int height, const std::string& title, const std::string& fontDir) {
    glfwSetErrorCallback(ErrorCallback);
    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // enable multisampling
    glfwWindowHint(GLFW_SAMPLES, 4);

    m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_Window) {
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    SetupGl();

    // Axis
    {
        m_Axis = std::make_unique<PureAxis>();
        if (!m_Axis->Initialize(/*axisLength=*/200.0f, /*headLen=*/20.0f, /*headWidth=*/10.0f)) {
            std::cerr << "Axis init failed" << std::endl;
            m_Axis.reset();
        }
    }

    // If a fontpath is provided add it
    if (!fontDir.empty()) {
        m_Gui.SetFontDirectory(fontDir);
    }
    if (!m_Gui.Initialize(m_Window, "#version 330")) return false;

    // Build Phong shader
    m_Shader = std::make_unique<PureShader>();
    m_Shader->BuildPhong();

    glfwGetFramebufferSize(m_Window, &m_FramebufferW, &m_FramebufferH);

    // Setup Cameras (currently only perspective camera)
    m_CameraPerspective = std::make_unique<PurePerspectiveCamera>();
    m_CameraPerspective->SetAspect((float)m_FramebufferW / std::max(1, m_FramebufferH));
    m_Camera = m_CameraPerspective.get();

    InstallGlfwCallbacks();

    m_Renderer = std::make_shared<PureRenderer>();

    return true;
}

void PureController::SetCameraMode(CameraMode mode) {
    if (mode == CameraMode::Perspective) {
        m_Camera = m_CameraPerspective.get();
    } else {
        cerr << "Other cameras are currently not supported yet!" << endl;
    }
    glfwGetFramebufferSize(m_Window, &m_FramebufferW, &m_FramebufferH);
    m_Camera->SetAspect((float)m_FramebufferW / std::max(1, m_FramebufferH));
}

void PureController::SetRightDockPanel(PanelRenderer panelRenderer) {
    m_RightPanel = panelRenderer;
}

void PureController::BeginDockspace() {
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImVec2 pos(vp->Pos.x + vp->Size.x - SIDEBAR_WIDTH, vp->Pos.y);
    ImVec2 size(SIDEBAR_WIDTH, vp->Size.y - 16.0);  // 16.0 means height of statusbar

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                             ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("RootDock", nullptr, flags);
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MainDock");
    ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
}

void PureController::EndDockspace() {
    ImGui::End();  // RootDock
}

void PureController::SetupGl() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);

    glClearColor(m_Background.r, m_Background.g, m_Background.b, 1.0f);
}

void PureController::InstallGlfwCallbacks() {
    glfwSetWindowUserPointer(m_Window, this);

    glfwSetKeyCallback(m_Window, [](GLFWwindow* w, int key, int sc, int action, int mods) {
        ImGui_ImplGlfw_KeyCallback(w, key, sc, action, mods);
        if (ImGui::GetIO().WantCaptureKeyboard) return;

        auto* self = static_cast<PureController*>(glfwGetWindowUserPointer(w));
        if (!self) return;

        // Handle internal shortcuts first
        //
        // FIT TO SCREEN
        if (key == GLFW_KEY_F && action == GLFW_PRESS) {
            PureBounds bounds;
            if (self->m_Scene->ComputeBounds(bounds)) {
                self->m_Camera->FitToBounds(bounds, 1.12f);
            }
        }
        // TOGGLE RIGHT PANEL
        else if (key == GLFW_KEY_E && action == GLFW_PRESS) {
            self->m_ShowRightPanel = !self->m_ShowRightPanel;
        }
        // Perspective camera
        else if (key == GLFW_KEY_HOME && action == GLFW_PRESS) {
            self->SetCameraMode(CameraMode::Perspective);
        }
        // Ortho camera
        else if (key == GLFW_KEY_END && action == GLFW_PRESS) {
            self->SetCameraMode(CameraMode::Ortho);
        }
        // Ortho camera
        else if (key == GLFW_KEY_E && action == GLFW_PRESS) {
            self->m_ShowRightPanel = !self->m_ShowRightPanel;
        }
        // EXIT APPLICATION
        else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(self->m_Window, true);
        }

        if (self->m_KeyPressedHandler && action == GLFW_PRESS) {
            self->m_KeyPressedHandler(key, mods);
        }
    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* w, int button, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(w, button, action, mods);
        if (ImGui::GetIO().WantCaptureMouse) return;

        auto* self = static_cast<PureController*>(glfwGetWindowUserPointer(w));
        if (!self) return;
        if (self->m_MouseButtonHandler) self->m_MouseButtonHandler(button, action, mods);
    });

    glfwSetScrollCallback(m_Window, [](GLFWwindow* w, double xoff, double yoff) {
        ImGui_ImplGlfw_ScrollCallback(w, xoff, yoff);
        if (ImGui::GetIO().WantCaptureMouse) return;

        auto* self = static_cast<PureController*>(glfwGetWindowUserPointer(w));
        if (!self) return;
        self->m_Camera->OnScrollWheel(yoff);
    });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* w, double x, double y) {
        ImGui_ImplGlfw_CursorPosCallback(w, x, y);
        if (ImGui::GetIO().WantCaptureMouse) return;
        auto* self = static_cast<PureController*>(glfwGetWindowUserPointer(w));
        if (!self) return;
        if (self->m_MouseMoveHandler) self->m_MouseMoveHandler(x, y);
    });

    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* w, int /*width*/, int /*height*/) {
        auto* self = static_cast<PureController*>(glfwGetWindowUserPointer(w));
        if (!self) return;
    });

    glfwSetWindowFocusCallback(
        m_Window, [](GLFWwindow* window, int focused) { ImGui_ImplGlfw_WindowFocusCallback(window, focused); });

    glfwSetWindowContentScaleCallback(m_Window, [](GLFWwindow* /*window*/, float xscale, float yscale) {
        ImGui::GetIO().DisplayFramebufferScale = ImVec2(xscale, yscale);
    });

    glfwSetCursorEnterCallback(
        m_Window, [](GLFWwindow* window, int entered) { ImGui_ImplGlfw_CursorEnterCallback(window, entered); });

    glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int c) { ImGui_ImplGlfw_CharCallback(window, c); });

    glfwSetMonitorCallback([](GLFWmonitor* monitor, int event) { ImGui_ImplGlfw_MonitorCallback(monitor, event); });
}

bool PureController::ShouldClose() const {
    return glfwWindowShouldClose(m_Window);
}

void PureController::BeginFrame() {
    HandleInput();

    m_Gui.Begin();
}

void PureController::DrawGui() {
    BeginDockspace();
    if (m_RightPanel && m_ShowRightPanel) {
        ImGui::SetNextWindowDockID(ImGui::GetID("MainDock"), ImGuiCond_Once);
        if (ImGui::Begin("Project", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav)) {
            m_RightPanel();
        }
        ImGui::End();
    }
    EndDockspace();

    // Statusbar
    auto now = std::chrono::steady_clock::now();
    bool showStatus =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - m_StatusTimestamp).count() < STATUSBAR_TIMEOUT_MS;

    char fps[64];
    snprintf(fps, sizeof(fps), "FPS: %.0f", ImGui::GetIO().Framerate);

    if (showStatus && !m_StatusMessage.empty()) {
        m_Gui.DrawStatusBar(m_StatusMessage, fps);
    } else {
        m_Gui.DrawStatusBar("", fps);
    }
}

void PureController::RenderScene(std::shared_ptr<PureScene> scene) {
    m_Scene = scene;
    Render();
}

void PureController::EndFrame() {
    m_Gui.End();

    glfwSwapBuffers(m_Window);
    glfwPollEvents();
}

void PureController::ToggleWireframe() {
    m_Wireframe = !m_Wireframe;
    m_Renderer->SetWireframe(m_Wireframe);
}

void PureController::Shutdown() {
    // Make sure that this sequence is correct!
    //
    if (m_Window) {
        glfwMakeContextCurrent(m_Window);
    } else {
        throw std::runtime_error("No window available anymore, you should not be here!");
    }

    m_Shader.reset();
    m_Scene->Clear();
    m_Scene.reset();
    m_Axis.reset();
    m_Renderer.reset();
    m_Gui.Shutdown();

    if (m_Window) {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }
    glfwTerminate();
}

void PureController::HandleInput() {
    double x, y;
    glfwGetCursorPos(m_Window, &x, &y);
    bool l = glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool r = glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    bool m = glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;

    if (l && m_Lmb) {
        m_Camera->Orbit((float)(x - m_LastX), (float)(y - m_LastY));
    }
    if ((r || m) && (m_Rmb || m_Mmb)) {
        m_Camera->Pan((float)(x - m_LastX), (float)(y - m_LastY));
    }

    m_Lmb = l;
    m_Rmb = r;
    m_Mmb = m;
    m_LastX = x;
    m_LastY = y;
}

void PureController::Render() {
    glfwGetFramebufferSize(m_Window, &m_FramebufferW, &m_FramebufferH);
    glViewport(0, 0, m_FramebufferW, m_FramebufferH);
    m_Camera->SetAspect((float)m_FramebufferW / std::max(1, m_FramebufferH));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = m_Camera->View();
    glm::mat4 proj = m_Camera->Projection();

    m_Renderer->DrawScene(m_Scene, m_Shader.get(), view, proj, m_Camera->Position(), m_Camera->ViewDirection());

    if (m_Axis) {
        m_Axis->Render(m_Camera->View(), m_Camera->Projection());
    }
}

void PureController::BuildDemoScene() {
    m_Scene->Clear();

    auto cube = PureMeshFactory::CreateCube(1.0f, 2.f);

    // Three colored cubes
    m_Scene->AddPart("cube 1", cube, glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)),
                     glm::vec3(1.0f, 0.25f, 0.25f));
    m_Scene->AddPart("cube 2", cube, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)),
                     glm::vec3(0.25f, 1.0f, 0.25f));
    m_Scene->AddPart("cube 3", cube, glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f)),
                     glm::vec3(0.25f, 0.25f, 1.0f));
}

void PureController::SetBackgroundColor(const glm::vec3& rgb) {
    m_Background = rgb;
}

void PureController::StoreBookmark(const std::string& name) {
    if (!m_Camera) return;
    CameraBookmark bm;
    bm.position = m_Camera->Position();
    bm.target = m_Camera->Target();
    bm.up = {0, 0, 1};
    bm.fovDeg = m_Camera->FovDeg();
    m_CameraBookmarks[name] = bm;
}

bool PureController::RecallBookmark(const std::string& name) {
    if (!m_Camera) return false;
    auto it = m_CameraBookmarks.find(name);
    if (it == m_CameraBookmarks.end()) return false;
    const auto& b = it->second;
    m_Camera->SetPosition(b.position);
    m_Camera->SetTarget(b.target);
    m_Camera->SetFovDeg(b.fovDeg);
    return true;
}

static bool writePixelsJPG(const std::string& path, int w, int h, const std::vector<uint8_t>& rgba, int quality) {
    // convert RGBA->RGB
    std::vector<uint8_t> rgb;
    rgb.reserve(w * h * 3);
    for (int i = 0; i < w * h; i++) {
        rgb.push_back(rgba[4 * i + 0]);
        rgb.push_back(rgba[4 * i + 1]);
        rgb.push_back(rgba[4 * i + 2]);
    }
    return stbi_write_jpg(path.c_str(), w, h, 3, rgb.data(), quality) != 0;
}

bool PureController::SaveScreenshotJPG(const std::string& path, int quality, int width, int height) {
    // 1) Offscreen FBO
    int w = width > 0 ? width : m_FramebufferW;
    int h = height > 0 ? height : m_FramebufferH;

    GLuint fbo = 0, tex = 0, rbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &tex);
        glDeleteRenderbuffers(1, &rbo);
        return false;
    }

    // 2) Render
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    glViewport(0, 0, w, h);
    glClearColor(m_Background.r, m_Background.g, m_Background.b, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = m_Camera->View();
    glm::mat4 proj = m_Camera->Projection();
    m_Renderer->DrawScene(m_Scene, m_Shader.get(), view, proj, m_Camera->Position(), m_Camera->ViewDirection());

    if (m_Axis) {
        m_Axis->Render(m_Camera->View(), m_Camera->Projection());
    }

    std::vector<uint8_t> pixels(w * h * 4);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    // Flip vertically
    for (int y = 0; y < h / 2; ++y) {
        auto* a = pixels.data() + y * w * 4;
        auto* b = pixels.data() + (h - 1 - y) * w * 4;
        for (int i = 0; i < w * 4; i++) std::swap(a[i], b[i]);
    }

    // 4) Write
    bool ok = writePixelsJPG(path, w, h, pixels, quality);

    // 5) Cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);
    glDeleteRenderbuffers(1, &rbo);
    return ok;
}

}  // namespace pure
