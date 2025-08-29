#pragma once
#include <BridgeTypes.hpp>
#include <core/LuaEngine.hpp>
#include <core/io/Project.hpp>
#include <pure/PureMesh.hpp>
#include <pure/PureScene.hpp>
#include <unordered_map>

class RenderBridge {
   public:
    explicit RenderBridge(pure::PureScene* scene);

    // baut/aktualisiert genau EIN Part
    bool BuildPart(std::shared_ptr<LuaEngine> engine, const io::Part& part, PartVisual* outVisual, std::string* err);

    // fügt (oder aktualisiert) den Part im PureScene ein
    // merkt sich eine PartID->Node Zuordnung
    void UpsertPartNode(const PartVisual& pv);

    // löscht einen Part aus der Scene
    void RemovePartNode(const std::string& partId);

    // einmalig: Szene aus ganzem Projekt aufbauen
    bool BuildProject(LuaEngine& engine, const io::Project& p, std::string* err);

   private:
    std::shared_ptr<pure::PureMesh> CreatePureMesh(const MeshData& md);

   private:
    pure::PureScene* m_Scene{nullptr};
    struct NodeEntry {
        std::shared_ptr<pure::PureMesh> mesh;
        // std::shared_ptr<pure::PureNode> node;
    };
    std::unordered_map<std::string, NodeEntry> m_Index;  // partId → node+mesh
};
