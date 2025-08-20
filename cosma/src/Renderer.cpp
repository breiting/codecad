// clang-format off
#include <glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <core/Camera.hpp>
#include <core/Renderer.hpp>
#include <geometry/Mesh.hpp>
#include <scene/Scene.hpp>
#include <ui/GuiLayer.hpp>

Renderer::Renderer() : m_Wireframe(false) {
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, m_Wireframe ? GL_LINE : GL_FILL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

Renderer::~Renderer() {
}

void Renderer::ToggleWireframe() {
    m_Wireframe = !m_Wireframe;
    glPolygonMode(GL_FRONT_AND_BACK, m_Wireframe ? GL_LINE : GL_FILL);
}

void Renderer::Render(Camera *camera, Scene *scene, GuiLayer *gui) {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (gui) {
        gui->Begin();
    }

    if (!camera || !scene) return;
    scene->Render(camera->GetViewMatrix(), camera->GetProjectionMatrix());
}

void Renderer::Resize(int width, int height) {
    glViewport(0, 0, width, height);
}
