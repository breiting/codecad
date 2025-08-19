#pragma once

#include <core/Application.hpp>
#include <core/Camera.hpp>
#include <core/Renderer.hpp>
#include <memory>
#include <scene/Scene.hpp>
#include <ui/GuiLayer.hpp>

#include "CoreEngine.hpp"
#include "core/FileWatcher.hpp"
#include "io/Project.hpp"
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
    CosmaController(const std::filesystem::path& outDir);
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

    void LoadProject(const std::string& projectFileName) override;
    void LoadLuaPartByPath(const std::string& path);

   protected:
    void DrawGui();
    void SetStatusMessage(const std::string& msg);
    std::shared_ptr<MeshNode> LoadStl(const std::string& fileName, bool watch = false);

   private:
    struct PartRecord {
        io::Part meta;
        geometry::ShapePtr shape;
        std::shared_ptr<MeshNode> node;
        std::filesystem::path absoluteSourcePath;
    };
    // Helpers
    std::shared_ptr<MeshNode> BuildMeshNodeFromShape(const TopoDS_Shape& s, const std::string& colorHex);
    geometry::ShapePtr ApplyTransform(const geometry::ShapePtr& shape, const io::Transform& tr);

    void BuildOrRebuildPart(PartRecord& rec);  // Run Lua → shape → transform → mesh
    void RebuildAllParts();                    // für PROJECT_KEY

   private:
    static constexpr const char* PROJECT_KEY = "__project__";

    CoreEngine m_Engine;
    io::Project m_Project;

    std::unordered_map<std::string, PartRecord> m_PartsByName;    // name -> record
    std::unordered_map<std::string, std::string> m_SourceToPart;  // abs source path -> part name

    std::filesystem::path m_ProjectRoot;
    std::filesystem::path m_Outdir;

    std::unique_ptr<Renderer> m_Renderer;
    std::unique_ptr<Camera> m_Camera;
    std::unique_ptr<Scene> m_Scene;
    Window* m_Window;

    std::unordered_map<std::string, FileWatcher> m_FileWatcher;

    float m_TargetT = 0.0f;

    // GUI
    std::unique_ptr<GuiLayer> m_Gui;
    std::string m_StatusMessage;
    std::chrono::steady_clock::time_point m_StatusTimestamp;

    bool m_LeftMouseButtonPressed;
    bool m_RightMouseButtonPressed;
    bool m_ShiftPressed;
};
