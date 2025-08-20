#pragma once

class InputHandler {
   public:
    virtual ~InputHandler() = default;

    virtual void OnFramebufferSize(int width, int height) = 0;
    virtual void OnMouseMove(double xpos, double ypos) = 0;
    virtual void OnScroll(double xoffset, double yoffset) = 0;
    virtual void OnKeyPressed(int key, int mods) = 0;
    virtual void OnKeyReleased(int key, int mods) = 0;
    virtual void OnMouseButtonPressed(int button, int mods) = 0;
    virtual void OnMouseButtonReleased(int button, int mods) = 0;
};
