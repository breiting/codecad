#pragma once
#include <imgui.h>

#include <core/Window.hpp>

class GuiLayer {
   public:
    void Init(Window *window);
    void Begin();
    void End();
    void Shutdown();

   private:
    Window *m_Window;
};
