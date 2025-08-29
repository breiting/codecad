#pragma once

class PureInputHandler {
   public:
    virtual ~PureInputHandler() = default;

    virtual void OnKeyPressed(int key, int mods) = 0;
    virtual void OnKeyReleased(int key, int mods) = 0;
    virtual void OnMouseButtonPressed(int button, int mods) = 0;
    virtual void OnMouseButtonReleased(int button, int mods) = 0;
};
