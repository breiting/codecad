#include <RenderBridge.hpp>
#include <glm/gtc/type_ptr.hpp>

RenderBridge::RenderBridge(pure::PureScene* scene) : m_Scene(scene) {
}

static MeshData TriangulateFromCore(LuaEngine& engine) {
    MeshData md;
    const auto tri = engine.GetTriangulation();  // ← nimm deine Core-API
    md.positions.reserve(tri.vertices.size() * 3);
    md.indices.reserve(tri.indices.size());

    for (const auto& v : tri.vertices) {
        md.positions.push_back(v.pos.x);
        md.positions.push_back(v.pos.y);
        md.positions.push_back(v.pos.z);
        // if you have normals: md.normals...
    }
    for (auto i : tri.indices) md.indices.push_back(i);
    return md;
}

bool RenderBridge::BuildPart(CoreEngine& engine, const io::Part& part, PartVisual* outVisual, std::string* err) {
    // 1) Lua ausführen → emit shape → triangulate (deine bestehende Pipeline)
    std::string runErr;
    if (!engine.RunFile(part.source, &runErr)) {
        if (err) *err = "Lua error in " + part.source + ": " + runErr;
        return false;
    }

    // 2) Triangulate aus Core holen
    MeshData md = TriangulateFromCore(engine);

    // 3) Visual füllen
    outVisual->id = part.id;
    outVisual->name = part.name;
    outVisual->mesh = std::move(md);
    outVisual->transform = io::ToMat4(part.transform);  // nimm deine Helper
    outVisual->visible = part.visible;
    outVisual->color = io::LookupMaterialColor(part.material);  // oder fallback

    return true;
}

std::shared_ptr<pure::PureMesh> RenderBridge::CreatePureMesh(const MeshData& md) {
    auto pm = std::make_shared<pure::PureMesh>();
    pm->SetData(md.positions, md.indices, md.normals);  // deine PureMesh API
    pm->Upload();
    return pm;
}

void RenderBridge::UpsertPartNode(const PartVisual& pv) {
    auto it = m_Index.find(pv.id);
    if (it == m_Index.end()) {
        // neu
        auto mesh = CreatePureMesh(pv.mesh);
        auto node = std::make_shared<pure::PureNode>(mesh);
        node->SetLocalTransform(pv.transform);
        node->SetColor(pv.color);
        node->SetVisible(pv.visible);
        m_Scene->AddNode(node);
        m_Index[pv.id] = {mesh, node};
    } else {
        // update existierend
        it->second.mesh->SetData(pv.mesh.positions, pv.mesh.indices, pv.mesh.normals);
        it->second.mesh->Upload();
        it->second.node->SetLocalTransform(pv.transform);
        it->second.node->SetColor(pv.color);
        it->second.node->SetVisible(pv.visible);
    }
}

void RenderBridge::RemovePartNode(const std::string& partId) {
    auto it = m_Index.find(partId);
    if (it == m_Index.end()) return;
    m_Scene->RemoveNode(it->second.node);
    m_Index.erase(it);
}

bool RenderBridge::BuildProject(CoreEngine& engine, const io::Project& p, std::string* err) {
    for (const auto& part : p.parts) {
        PartVisual pv;
        if (!BuildPart(engine, part, &pv, err)) return false;
        UpsertPartNode(pv);
    }
    return true;
}
