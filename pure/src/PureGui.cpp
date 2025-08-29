#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <filesystem>
#include <iostream>
#include <pure/PureGui.hpp>

namespace fs = std::filesystem;

namespace pure {

PureGui::~PureGui() {
    Shutdown();
}

void PureGui::SetFontDirectory(const std::string& fontDir) {
    m_FontDir = fontDir;
}

bool PureGui::Initialize(GLFWwindow* window, const char* glslVersion) {
    if (m_Initialized) return true;
    if (!window) return false;

    m_Window = window;
    if (glslVersion && *glslVersion) m_GlslVersion = glslVersion;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    // Backends init
    if (!ImGui_ImplGlfw_InitForOpenGL(m_Window, true)) return false;
    if (!ImGui_ImplOpenGL3_Init(m_GlslVersion.c_str())) return false;

    // Fonts (optional)
    LoadDefaultFonts();

    m_Initialized = true;
    return true;
}

void PureGui::Begin() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void PureGui::End() {
    // Make sure viewport matches current framebuffer
    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(m_Window, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void PureGui::Shutdown() {
    if (!m_Initialized) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    m_Window = nullptr;
    m_Initialized = false;
}

void PureGui::LoadDefaultFonts() {
    if (m_FontDir.empty()) return;

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    const fs::path roboto = fs::path(m_FontDir) / "Roboto-Medium.ttf";
    if (!fs::exists(roboto)) {
        std::cerr << "PureGui: Roboto-Medium.ttf not found in " << m_FontDir << "\n";
        return;
    }

    ImFont* def = io.Fonts->AddFontFromFileTTF(roboto.string().c_str(), 16.0f);
    if (def) io.FontDefault = def;
}

void PureGui::DrawStatusBar(const std::string& leftText, const std::string& rightText) {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    const float barHeight = ImGui::GetFrameHeight() + 6.0f;

    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, vp->Pos.y + vp->Size.y - barHeight));
    ImGui::SetNextWindowSize(ImVec2(vp->Size.x, barHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground;

    ImGui::Begin("StatusBar", nullptr, flags);

    // Left text
    ImGui::TextUnformatted(leftText.c_str());

    // Right text aligned to the right
    float rightWidth = ImGui::CalcTextSize(rightText.c_str()).x;
    float fullWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
    float padRight = 10.0f;
    ImGui::SameLine(fullWidth - rightWidth - padRight);
    ImGui::TextUnformatted(rightText.c_str());

    ImGui::End();
}

}  // namespace pure
