// clang-format off
#include <glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <core/RenderEngine.hpp>
#include <iostream>

GLuint g_DefaultTexture = 0;

/**
 * Creates a default texture which can be used, if no textures are used for shaders
 */
void CreateDefaultTexture() {
    unsigned char whitePixel[4] = {255, 255, 255, 255};  // RGBA white

    glGenTextures(1, &g_DefaultTexture);
    glBindTexture(GL_TEXTURE_2D, g_DefaultTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

RenderEngine::RenderEngine(std::unique_ptr<Application> app) : m_Application(std::move(app)) {
}

bool RenderEngine::Init(int width, int height, const std::string& title) {
    m_Window = std::make_unique<Window>(width, height, title);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return false;
    }

    CreateDefaultTexture();

    m_Application->Init(m_Window.get());

    m_Window->SetInputHandler(m_Application.get());

    lastTime = static_cast<float>(glfwGetTime());
    return true;
}

void RenderEngine::Run() {
    while (!m_Window->ShouldClose()) {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // needs to be called before rendering due to ImGui
        m_Window->PollEvents();

        m_Application->Update(deltaTime);
        m_Application->Render();
        m_Window->SwapBuffers();
    }
    m_Application->Shutdown();
    m_Window->Close();
}
