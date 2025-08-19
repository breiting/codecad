// clang-format off
#include <glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <imgui.h>
#include <imgui_impl_glfw.h>

#include <core/Window.hpp>
#include <stdexcept>

static InputHandler* s_InputHandler = nullptr;
void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
    if (s_InputHandler) s_InputHandler->OnFramebufferSize(width, height);
}

void mouse_callback(GLFWwindow* /*window*/, double xpos, double ypos) {
    if (s_InputHandler) s_InputHandler->OnMouseMove(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // Handle ImGUI first
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    if (ImGui::GetIO().WantCaptureMouse) return;

    if (s_InputHandler) s_InputHandler->OnScroll(xoffset, yoffset);
}

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Handle ImGUI first
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    if (s_InputHandler) {
        if (action == GLFW_PRESS) {
            s_InputHandler->OnKeyPressed(key, mods);
        } else if (action == GLFW_RELEASE) {
            s_InputHandler->OnKeyReleased(key, mods);
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    // Handle ImGUI first
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) return;

    if (s_InputHandler) {
        if (action == GLFW_PRESS) {
            s_InputHandler->OnMouseButtonPressed(button, mods);
        } else if (action == GLFW_RELEASE) {
            s_InputHandler->OnMouseButtonReleased(button, mods);
        }
    }
}

Window::Window(int width, int height, const std::string& title) : m_Width(width), m_Height(height) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_Window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, GLFW_TRUE);
    glfwMakeContextCurrent(m_Window);

    // GLFW to capture all mouse movements
    // glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glViewport(0, 0, width, height);
}

Window::~Window() {
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Window::SetInputHandler(InputHandler* handler) {
    s_InputHandler = handler;
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
    glfwSetCursorPosCallback(m_Window, mouse_callback);
    glfwSetScrollCallback(m_Window, scroll_callback);
    glfwSetKeyCallback(m_Window, keyboard_callback);
    glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
}

void Window::PollEvents() {
    glfwPollEvents();
}

void Window::SwapBuffers() {
    glfwSwapBuffers(m_Window);
}

void Window::Close() {
    glfwSetWindowShouldClose(m_Window, true);
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_Window);
}
