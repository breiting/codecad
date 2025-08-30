#pragma once

// clang-format off
#include <glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <functional>
#include <pure/PureCamera.hpp>
#include <pure/PureScene.hpp>
#include <pure/PureShader.hpp>

#include "pure/PureAxis.hpp"
#include "pure/PureGui.hpp"
#include "pure/PureRenderer.hpp"

namespace pure {

class PureController {
   public:
    using PanelRenderer = std::function<void()>;

    PureController();
    ~PureController();

    bool Initialize(int width, int height, const std::string& title);
    void Run(std::shared_ptr<PureScene> scene);
    void Shutdown();

    void BuildDemoScene();

    void ToggleWireframe();
    void SetStatus(const std::string& msg);
    void SetRightDockPanel(PanelRenderer panelRenderer);

    using KeyPressedHandler = std::function<void(int key, int mods)>;
    using MouseButtonHandler = std::function<void(int button, int action, int mods)>;

    void SetKeyPressedHandler(KeyPressedHandler h) {
        m_KeyPressedHandler = std::move(h);
    }
    void SetMouseButtonHandler(MouseButtonHandler h) {
        m_MouseButtonHandler = std::move(h);
    }

   private:
    void SetupGl();
    void HandleInput();
    void Render();
    void InstallGlfwCallbacks();
    void BeginDockspace();
    void EndDockspace();

   private:
    GLFWwindow* m_Window = nullptr;

    PanelRenderer m_RightPanel;

    // Handler
    KeyPressedHandler m_KeyPressedHandler;
    MouseButtonHandler m_MouseButtonHandler;

    // Input-Cache (mouse)
    bool m_Lmb = false, m_Rmb = false, m_Mmb = false;
    double m_LastX = 0.0, m_LastY = 0.0;

    PureCamera m_Camera;
    PureShader m_Shader;
    std::shared_ptr<PureScene> m_Scene;
    std::shared_ptr<PureRenderer> m_Renderer;

    PureGui m_Gui;

    std::unique_ptr<PureAxis> m_Axis;

    int m_FramebufferW = 1280;
    int m_FramebufferH = 800;

    bool m_Wireframe = false;
};

}  // namespace pure
