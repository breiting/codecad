#include "io/Project.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

using json = nlohmann::json;

static std::string readFile(const std::string& p) {
    std::ifstream f(p);
    if (!f) throw std::runtime_error("LoadProject: cannot open file: " + p);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static glm::vec2 getVec2(const json& j, const char* key) {
    glm::vec2 v{};
    if (j.contains(key) && j[key].is_array() && j[key].size() >= 2) {
        v.x = j[key][0].get<double>();
        v.y = j[key][1].get<double>();
    }
    return v;
}
static glm::vec3 getVec3(const json& j, const char* key) {
    glm::vec3 v{};
    if (j.contains(key) && j[key].is_array() && j[key].size() >= 3) {
        v.x = j[key][0].get<double>();
        v.y = j[key][1].get<double>();
        v.z = j[key][2].get<double>();
    }
    return v;
}

static Transform getTransform(const json& jt) {
    Transform t{};
    if (jt.is_null()) return t;
    t.translate = getVec3(jt, "translate");
    t.rotate = getVec3(jt, "rotate");
    t.scale = getVec3(jt, "scale");
    if (t.scale.x == 0) t.scale.x = 1;
    if (t.scale.y == 0) t.scale.y = 1;
    if (t.scale.z == 0) t.scale.z = 1;
    return t;
}

Project LoadProject(const std::string& path) {
    const auto txt = readFile(path);
    json j = json::parse(txt);

    Project p{};
    p.version = j.value("version", 1);

    // meta
    if (j.contains("meta")) {
        const auto& m = j["meta"];
        p.meta.name = m.value("name", "");
        p.meta.author = m.value("author", "");
        p.meta.units = m.value("units", "mm");
    }

    // viewer
    if (j.contains("viewer")) {
        const auto& v = j["viewer"];
        p.viewer.bg = v.value("bg", "");
        p.viewer.grid = v.value("grid", true);
    }

    // workarea
    if (j.contains("workarea")) {
        const auto& b = j["workarea"];
        p.workarea.size = getVec2(b, "size");
    }

    // materials
    if (j.contains("materials") && j["materials"].is_object()) {
        for (auto it = j["materials"].begin(); it != j["materials"].end(); ++it) {
            Material m{};
            m.color = it.value().value("color", "");
            p.materials[it.key()] = m;
        }
    }

    // parts
    if (j.contains("parts") && j["parts"].is_array()) {
        for (const auto& jp : j["parts"]) {
            Part pr{};
            pr.id = jp.value("id", "");
            pr.name = jp.value("name", "");
            pr.stl = jp.value("stl", "");
            pr.source = jp.value("source", "");
            pr.material = jp.value("material", "");
            pr.transform = getTransform(jp.value("transform", json::object()));
            p.parts.emplace_back(std::move(pr));
        }
    }

    // animations
    if (j.contains("animations") && j["animations"].is_object()) {
        for (auto it = j["animations"].begin(); it != j["animations"].end(); ++it) {
            Animation anim{};
            anim.component = it.value().value("component", "");
            anim.type = it.value().value("type", "");
            anim.from = getVec3(it.value(), "from");
            anim.to = getVec3(it.value(), "to");
        }
    }

    return p;
}
