#pragma once
#include <core/InputHandler.hpp>

#include "io/Project.hpp"

class Renderer;
class Window;

class Application : public InputHandler {
   public:
    virtual ~Application() = default;

    virtual void Init(Window* window) = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void Shutdown() = 0;

    virtual void LoadProject(const std::string& projectFileName) = 0;

    // InputHandler
    virtual void OnFramebufferSize(int width, int height) = 0;
    virtual void OnMouseMove(double xpos, double ypos) = 0;
    virtual void OnScroll(double xoffset, double yoffset) = 0;
    virtual void OnKeyPressed(int key, int mods) = 0;
    virtual void OnKeyReleased(int key, int mods) = 0;
    virtual void OnMouseButtonPressed(int button, int mods) = 0;
    virtual void OnMouseButtonReleased(int button, int mods) = 0;
};
