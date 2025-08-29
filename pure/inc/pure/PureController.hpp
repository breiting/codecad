#pragma once

// clang-format off
#include <glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <pure/PureCamera.hpp>
#include <pure/PureScene.hpp>
#include <pure/PureShader.hpp>

#include "pure/PureAxis.hpp"
#include "pure/PureGui.hpp"

namespace pure {

class PureController {
   public:
    PureController();
    ~PureController();

    bool Initialize(int width, int height, const std::string& title);
    void Run();
    void Shutdown();

    // Demo scene
    void BuildDemoScene();

   private:
    void SetupGl();
    void HandleInput();
    void Frame();

   private:
    GLFWwindow* m_Window = nullptr;

    // Input-Cache (mouse)
    bool m_Lmb = false, m_Rmb = false, m_Mmb = false;
    double m_LastX = 0.0, m_LastY = 0.0;

    PureCamera m_Camera;
    PureScene m_Scene;
    PureShader m_Shader;

    PureGui m_Gui;

    std::unique_ptr<PureAxis> m_Axis;

    int m_FramebufferW = 1280;
    int m_FramebufferH = 800;
};

}  // namespace pure
