#pragma once

#include <core/Application.hpp>
#include <core/Window.hpp>
#include <memory>

class RenderEngine {
   public:
    RenderEngine(std::unique_ptr<Application> app);

    bool Init(int width, int height, const std::string& title);
    void Run();

    Application* GetApp() {
        return m_Application.get();
    }

   private:
    std::unique_ptr<Application> m_Application;
    std::unique_ptr<Window> m_Window;

    float lastTime = 0.0f;
};
