#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <pure/PureController.hpp>
#include <pure/PureMeshFactory.hpp>
#include <pure/PureRenderArea.hpp>
#include <pure/PureRenderer.hpp>

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "pure/PureCamera.hpp"

namespace pure {

const float NEAR_PLANE = 0.01f;
const float FAR_PLANE = 5000.0f;

static PureCamera* sCamera = nullptr;

// GLFW Callbacks
static void ScrollCallback(GLFWwindow* /*win*/, double /*xoff*/, double yoff) {
    if (sCamera) sCamera->OnScrollWheel(yoff);
}
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
    glfwSetScrollCallback(m_Window, ScrollCallback);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    SetupGl();

    sCamera = &m_Camera;

    // Axis
    {
        m_Axis = std::make_unique<PureAxis>();
        std::string err;
        if (!m_Axis->Initialize(/*axisLength=*/200.0f, /*headLen=*/20.0f, /*headWidth=*/10.0f, &err)) {
            std::cerr << "Axis init failed: " << err << "\n";
            m_Axis.reset();
        }
        // Optional scale or translate
        // m_Axis->SetModel(glm::translate(glm::mat4(1), glm::vec3(0,0,0)));
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

    return true;
}

void PureController::SetupGl() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.11f, 0.12f, 0.14f, 1.0f);
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

    double sy = 0.0;
    if (glfwGetKey(m_Window, GLFW_KEY_EQUAL) == GLFW_PRESS) sy += 1.0;
    if (glfwGetKey(m_Window, GLFW_KEY_MINUS) == GLFW_PRESS) sy -= 1.0;
    if (std::abs(sy) > 1e-6) m_Camera.Dolly(sy);

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

    DrawScene(m_Scene, m_Shader, view, proj, m_Camera.Position(), m_Camera.ViewDirection());

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
