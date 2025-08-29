#pragma once
// clang-format off
#include <glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <string>

namespace pure {

class PureGui {
   public:
    PureGui() = default;
    ~PureGui();

    // Create ImGui context + init backends
    bool Initialize(GLFWwindow* window, const char* glslVersion = "#version 330");

    // Optional: set directory containing Roboto-Medium.ttf before Initialize()
    void SetFontDirectory(const std::string& fontDir);

    // Per-frame
    void Begin();
    void End();

    // Simple fixed status bar at the bottom of the main viewport
    void DrawStatusBar(const std::string& leftText, const std::string& rightText);

    // Destroy backends + context
    void Shutdown();

   private:
    void LoadDefaultFonts();  // tries to load Roboto-Medium.ttf from m_FontDir

   private:
    GLFWwindow* m_Window = nullptr;
    bool m_Initialized = false;
    std::string m_GlslVersion = "#version 330";
    std::string m_FontDir;  // directory that contains Roboto-Medium.ttf
};

}  // namespace pure
