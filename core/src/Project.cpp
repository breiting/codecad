#include "io/Project.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

using json = nlohmann::json;

static std::string read_file(const std::string& p) {
    std::ifstream f(p);
    if (!f) throw std::runtime_error("LoadProject: cannot open file: " + p);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static glm::vec2 get_vec2(const json& j, const char* key) {
    glm::vec2 v{};
    if (j.contains(key) && j[key].is_array() && j[key].size() >= 2) {
        v.x = j[key][0].get<double>();
        v.y = j[key][1].get<double>();
    }
    return v;
}
static glm::vec3 get_vec3(const json& j, const char* key) {
    glm::vec3 v{};
    if (j.contains(key) && j[key].is_array() && j[key].size() >= 3) {
        v.x = j[key][0].get<double>();
        v.y = j[key][1].get<double>();
        v.z = j[key][2].get<double>();
    }
    return v;
}

static Transform get_transform(const json& jt) {
    Transform t{};
    if (jt.is_null()) return t;
    t.translate = get_vec3(jt, "translate");
    t.rotate = get_vec3(jt, "rotate");
    t.scale = get_vec3(jt, "scale");
    if (t.scale.x == 0) t.scale.x = 1;
    if (t.scale.y == 0) t.scale.y = 1;
    if (t.scale.z == 0) t.scale.z = 1;
    return t;
}

Project LoadProject(const std::string& path) {
    const auto txt = read_file(path);
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
        p.viewer.explode = v.value("explode", 0.0);
    }

    // bed
    if (j.contains("bed")) {
        const auto& b = j["bed"];
        p.bed.type = b.value("type", "");
        p.bed.size = get_vec2(b, "size");
        p.bed.origin = b.value("origin", "");
    }

    // materials
    if (j.contains("materials") && j["materials"].is_object()) {
        for (auto it = j["materials"].begin(); it != j["materials"].end(); ++it) {
            Material m{};
            m.color = it.value().value("color", "");
            m.density = it.value().value("density", 0.0);
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
            pr.color = jp.value("color", "");
            pr.transform = get_transform(jp.value("transform", json::object()));
            pr.explode = get_vec3(jp, "explode");
            if (jp.contains("bed_place") && jp["bed_place"].is_object()) {
                pr.bed_place.xy = get_vec2(jp["bed_place"], "xy");
                pr.bed_place.angle = jp["bed_place"].value("angle", 0.0);
                pr.bed_place.has = true;
            }
            if (jp.contains("tags") && jp["tags"].is_array()) {
                for (const auto& t : jp["tags"]) pr.tags.emplace_back(t.get<std::string>());
            }
            if (jp.contains("meta") && jp["meta"].is_object()) {
                for (auto it = jp["meta"].begin(); it != jp["meta"].end(); ++it) {
                    if (it.value().is_string())
                        pr.meta[it.key()] = it.value().get<std::string>();
                    else
                        pr.meta[it.key()] = it.value().dump();
                }
            }
            p.parts.emplace_back(std::move(pr));
        }
    }

    // animations (not yet supported and optional!)

    // bom (optional)
    if (j.contains("bom") && j["bom"].contains("items") && j["bom"]["items"].is_array()) {
        for (const auto& bi : j["bom"]["items"]) {
            BOMItem it{};
            it.name = bi.value("name", "");
            it.qty = bi.value("qty", 1);
            it.material = bi.value("material", "");
            it.dim = bi.value("dim", "");
            it.note = bi.value("note", "");
            p.bom.emplace_back(std::move(it));
        }
    }

    return p;
}
