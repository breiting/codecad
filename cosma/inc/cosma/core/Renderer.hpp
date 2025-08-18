#pragma once

#include <geometry/Mesh.hpp>
#include <glm/glm.hpp>
#include <material/Shader.hpp>

class Camera;
class GuiLayer;
class Scene;

class Renderer {
   public:
    Renderer();
    ~Renderer();

    void Render(Camera *camera, Scene *scene, GuiLayer *gui = 0);
    void Resize(int width, int height);
    void ToggleWireframe();

   private:
    bool m_Wireframe;
};
