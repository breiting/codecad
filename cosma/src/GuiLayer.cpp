// clang-format off
#include <glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <ui/GuiLayer.hpp>

#include "io/Paths.hpp"

namespace fs = std::filesystem;

void LoadDefaultFonts(const std::string& fontDir) {
    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->Clear();

    const std::string roboto = fontDir + "/Roboto-Medium.ttf";

    if (!fs::exists(roboto)) {
        std::cerr << "Roboto-Medium font does not exist" << std::endl;
        return;
    }
    ImFont* def = io.Fonts->AddFontFromFileTTF(roboto.c_str(), 16.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
    if (def) io.FontDefault = def;

    ImGui_ImplOpenGL3_DestroyFontsTexture();  // idempotent
    ImGui_ImplOpenGL3_CreateFontsTexture();
}

void GuiLayer::Init(Window* window) {
    if (!window) return;
    m_Window = window;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window->GetNativeWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    LoadDefaultFonts(io::DefaultInstallFontsPath());
}

void GuiLayer::Begin() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GuiLayer::End() {
    int width, height;
    glfwGetFramebufferSize(m_Window->GetNativeWindow(), &width, &height);
    glViewport(0, 0, width, height);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GuiLayer::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
