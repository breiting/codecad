#pragma once

#include <core/Application.hpp>
#include <core/Camera.hpp>
#include <core/Renderer.hpp>
#include <memory>
#include <scene/Scene.hpp>
#include <ui/GuiLayer.hpp>

#include "core/FileWatcher.hpp"
#include "scene/MeshNode.hpp"

class Camera;
class Light;
class MeshNode;

struct NodeRef {
    std::string name;
    std::shared_ptr<MeshNode> node;
    glm::vec3 basePos{0.0f};  // assembled base
};

class CosmaController : public Application {
   public:
    CosmaController();
    void Init(Window* window) override;
    void Update(float deltaTime) override;
    void Render() override;
    void Shutdown() override;

    // InputHandler
    void OnFramebufferSize(int width, int height) override;
    void OnMouseMove(double xpos, double ypos) override;
    void OnScroll(double xoffset, double yoffset) override;
    void OnKeyPressed(int key, int mods) override;
    void OnKeyReleased(int key, int mods) override;
    void OnMouseButtonPressed(int button, int mods) override;
    void OnMouseButtonReleased(int button, int mods) override;

    void LoadProject(const Project& project) override;

   protected:
    void DrawGui();
    void SetStatusMessage(const std::string& msg);
    void LoadManifest(const std::string& fileName);
    std::shared_ptr<MeshNode> LoadStl(const std::string& fileName, bool watch = false);

   private:
    std::unique_ptr<Renderer> m_Renderer;
    std::unique_ptr<Camera> m_Camera;
    std::unique_ptr<Scene> m_Scene;
    std::shared_ptr<MeshNode> m_MeshNode;
    Window* m_Window;

    std::unique_ptr<FileWatcher> m_FileWatcher;

    // Manifest Mode
    std::vector<NodeRef> m_NodeRefs;
    bool m_IsManifestMode = false;
    float m_TargetT = 0.0f;

    // GUI
    std::unique_ptr<GuiLayer> m_Gui;
    std::string m_StatusMessage;
    std::chrono::steady_clock::time_point m_StatusTimestamp;

    bool m_LeftMouseButtonPressed;
    bool m_RightMouseButtonPressed;
    bool m_ShiftPressed;
};
