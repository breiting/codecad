#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <pure/PureController.hpp>
#include <pure/PureMeshFactory.hpp>
#include <pure/PureRenderArea.hpp>
#include <pure/PureRenderer.hpp>

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "pure/PureCamera.hpp"

namespace pure {

const float NEAR_PLANE = 0.01f;
const float FAR_PLANE = 5000.0f;

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

bool PureController::Initialize(int width, int height, const std::string& title) {
    glfwSetErrorCallback(ErrorCallback);
    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

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
        std::string err;
        if (!m_Axis->Initialize(/*axisLength=*/200.0f, /*headLen=*/20.0f, /*headWidth=*/10.0f, &err)) {
            std::cerr << "Axis init failed: " << err << "\n";
            m_Axis.reset();
        }
    }

    // TODO: fix path
    m_Gui.SetFontDirectory("/Users/breiting/workspace/codecad/assets/fonts");

    if (!m_Gui.Initialize(m_Window, "#version 330")) return false;

    std::string err;
    if (!m_Shader.BuildPhong(&err)) {
        std::cerr << "Shader error: " << err << "\n";
        return false;
    }

    glfwGetFramebufferSize(m_Window, &m_FramebufferW, &m_FramebufferH);
    m_Camera.SetAspect((float)m_FramebufferW / std::max(1, m_FramebufferH));

    InstallGlfwCallbacks();

    m_Renderer = std::make_shared<PureRenderer>();

    return true;
}

void PureController::SetupGl() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.11f, 0.12f, 0.14f, 1.0f);
}

void PureController::InstallGlfwCallbacks() {
    glfwSetWindowUserPointer(m_Window, this);

    glfwSetKeyCallback(m_Window, [](GLFWwindow* w, int key, int sc, int action, int mods) {
        ImGui_ImplGlfw_KeyCallback(w, key, sc, action, mods);
        if (ImGui::GetIO().WantCaptureKeyboard) return;

        auto* self = static_cast<PureController*>(glfwGetWindowUserPointer(w));
        if (!self) return;

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
        self->m_Camera.OnScrollWheel(yoff);
    });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* w, double /*x*/, double /*y*/) {
        auto* self = static_cast<PureController*>(glfwGetWindowUserPointer(w));
        if (!self) return;
    });

    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* w, int /*width*/, int /*height*/) {
        auto* self = static_cast<PureController*>(glfwGetWindowUserPointer(w));
        if (!self) return;
    });
}

void PureController::Run(std::shared_ptr<PureScene> scene) {
    m_Scene = scene;
    // Fit camera
    glm::vec3 bmin, bmax;
    if (scene->ComputeBounds(bmin, bmax)) {
        m_Camera.FitToBounds(bmin, bmax, 1.12f);
    }

    while (!glfwWindowShouldClose(m_Window)) {
        HandleInput();

        m_Gui.Begin();

        char fps[64];
        snprintf(fps, sizeof(fps), "FPS: %.0f", ImGui::GetIO().Framerate);
        m_Gui.DrawStatusBar("Ready.", fps);

        Render();

        m_Gui.End();

        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
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
    }

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

    if (l && m_Lmb) m_Camera.Orbit((float)(x - m_LastX), (float)(y - m_LastY));
    if ((r || m) && (m_Rmb || m_Mmb)) m_Camera.Pan((float)(x - m_LastX), (float)(y - m_LastY));

    m_Lmb = l;
    m_Rmb = r;
    m_Mmb = m;
    m_LastX = x;
    m_LastY = y;

    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_Window, true);
    }

    if (glfwGetKey(m_Window, GLFW_KEY_F) == GLFW_PRESS) {
        glm::vec3 bmin, bmax;
        if (m_Scene->ComputeBounds(bmin, bmax)) m_Camera.FitToBounds(bmin, bmax, 1.12f);
    }
}

void PureController::Render() {
    glfwGetFramebufferSize(m_Window, &m_FramebufferW, &m_FramebufferH);
    glViewport(0, 0, m_FramebufferW, m_FramebufferH);
    m_Camera.SetAspect((float)m_FramebufferW / std::max(1, m_FramebufferH));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = m_Camera.View();
    glm::mat4 proj = m_Camera.Projection(NEAR_PLANE, FAR_PLANE);

    m_Renderer->DrawScene(m_Scene, m_Shader, view, proj, m_Camera.Position(), m_Camera.ViewDirection());

    if (m_Axis) {
        m_Axis->Render(m_Camera.View(), m_Camera.Projection(NEAR_PLANE, FAR_PLANE));
    }
}

void PureController::BuildDemoScene() {
    m_Scene->Clear();

    auto cube = PureMeshFactory::CreateCube(1.0f, 2.f);

    // Three colored cubes
    m_Scene->AddPart(cube, glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)),
                     glm::vec3(1.0f, 0.25f, 0.25f));
    m_Scene->AddPart(cube, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)), glm::vec3(0.25f, 1.0f, 0.25f));
    m_Scene->AddPart(cube, glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f)), glm::vec3(0.25f, 0.25f, 1.0f));
}

}  // namespace pure
