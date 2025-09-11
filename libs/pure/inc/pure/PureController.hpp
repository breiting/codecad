#pragma once

#include <glad.h>

#include <chrono>
#include <functional>
#include <pure/IPureCamera.hpp>
#include <pure/PureScene.hpp>
#include <pure/PureShader.hpp>

#include "pure/IPureCamera.hpp"
#include "pure/PureAxis.hpp"
#include "pure/PureGui.hpp"
#include "pure/PurePerspectiveCamera.hpp"
#include "pure/PureRenderer.hpp"

namespace pure {

enum class CameraMode { Perspective, Ortho };

class PureController {
   public:
    using PanelRenderer = std::function<void()>;

    PureController();
    ~PureController();

    bool Initialize(int width, int height, const std::string& title, const std::string& fontDir = "");
    void Shutdown();

    void BuildDemoScene();

    void ToggleWireframe();
    void SetStatus(const std::string& msg) {
        m_StatusMessage = msg;
        m_StatusTimestamp = std::chrono::steady_clock::now();
    }
    void SetRightDockPanel(PanelRenderer panelRenderer);

    // Event handler which can be registered
    using KeyPressedHandler = std::function<void(int key, int mods)>;
    using MouseButtonHandler = std::function<void(int button, int action, int mods)>;
    using MouseMoveHandler = std::function<void(double x, double y)>;

    void SetKeyPressedHandler(KeyPressedHandler h) {
        m_KeyPressedHandler = std::move(h);
    }
    void SetMouseButtonHandler(MouseButtonHandler h) {
        m_MouseButtonHandler = std::move(h);
    }

    void SetMouseMoveHandler(MouseMoveHandler h) {
        m_MouseMoveHandler = std::move(h);
    }

    bool ShouldClose() const;
    void BeginFrame();
    void DrawGui();
    void RenderScene(std::shared_ptr<PureScene> scene);
    void EndFrame();

    IPureCamera* Camera() {
        return m_Camera;
    }

    int GetFramebufferWidth() const {
        return m_FramebufferW;
    }
    int GetFramebufferHeight() const {
        return m_FramebufferH;
    }

    void SetBackgroundColor(const glm::vec3& rgb);
    void StoreBookmark(const std::string& name);
    bool RecallBookmark(const std::string& name);
    bool SaveScreenshotJPG(const std::string& path, int quality = 90, int width = 0, int height = 0);

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
    MouseMoveHandler m_MouseMoveHandler;

    // Input-Cache (mouse)
    bool m_Lmb = false, m_Rmb = false, m_Mmb = false;
    double m_LastX = 0.0, m_LastY = 0.0;

    // Camera (currently only perspective camera)
    std::unique_ptr<PurePerspectiveCamera> m_CameraPerspective;
    IPureCamera* m_Camera = nullptr;
    void SetCameraMode(CameraMode mode);

    std::unique_ptr<PureShader> m_Shader;
    std::shared_ptr<PureScene> m_Scene;
    std::shared_ptr<PureRenderer> m_Renderer;

    PureGui m_Gui;

    std::unique_ptr<PureAxis> m_Axis;

    int m_FramebufferW = 1600;
    int m_FramebufferH = 1200;

    bool m_Wireframe = false;
    bool m_ShowRightPanel = true;

    std::string m_StatusMessage = "Ready!";
    std::chrono::steady_clock::time_point m_StatusTimestamp;

    std::unordered_map<std::string, CameraBookmark> m_CameraBookmarks;
    glm::vec3 m_Background{0.11f, 0.12f, 0.14f};
};

}  // namespace pure
