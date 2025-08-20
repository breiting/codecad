#pragma once

#include <GLFW/glfw3.h>

#include <core/InputHandler.hpp>
#include <string>

class Window {
   public:
    Window(int width, int height, const std::string& title);
    ~Window();

    void PollEvents();
    void SwapBuffers();
    void Close();
    bool ShouldClose() const;

    void SetInputHandler(InputHandler* handler);

    GLFWwindow* GetNativeWindow() const {
        return m_Window;
    }

    int GetWidth() const {
        return m_Width;
    }

    int GetHeight() const {
        return m_Height;
    }

   private:
    GLFWwindow* m_Window;
    int m_Width;
    int m_Height;
};
