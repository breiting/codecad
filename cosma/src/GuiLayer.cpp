// clang-format off
#include <glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ui/GuiLayer.hpp>

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
