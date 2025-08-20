#include "io/Project.hpp"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

using json = nlohmann::json;

namespace io {

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

static json j_vec2(const glm::vec2& v) {
    return json::array({v.x, v.y});
}
static json j_vec3(const glm::vec3& v) {
    return json::array({v.x, v.y, v.z});
}

static json j_transform(const Transform& t) {
    json jt;
    jt["translate"] = j_vec3(t.translate);
    jt["rotate"] = j_vec3(t.rotate);
    jt["scale"] = t.scale;
    return jt;
}

static bool getBoolSafe(const nlohmann::json& j, const std::string& key, bool def = false) {
    if (!j.contains(key)) return def;
    try {
        if (j[key].is_boolean()) return j[key].get<bool>();
        if (j[key].is_number_integer()) return j[key].get<int>() != 0;
        if (j[key].is_string()) {
            std::string s = j[key].get<std::string>();
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return (s == "true" || s == "1" || s == "yes" || s == "on");
        }
    } catch (...) {
    }
    return def;
}

static Transform getTransform(const json& jt) {
    Transform t{};
    if (jt.is_null()) return t;
    t.translate = getVec3(jt, "translate");
    t.rotate = getVec3(jt, "rotate");
    t.scale = jt["scale"].get<double>();
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
            pr.source = jp.value("source", "");
            pr.material = jp.value("material", "");
            pr.transform = getTransform(jp.value("transform", json::object()));
            pr.visible = getBoolSafe(jp, "visible", true);
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
            p.animations[it.key()] = std::move(anim);
        }
    }

    return p;
}

bool SaveProject(const Project& p, const std::string& path, bool pretty) {
    json j;

    // version
    j["version"] = p.version;

    // meta
    {
        json m;
        m["name"] = p.meta.name;
        m["author"] = p.meta.author;
        m["units"] = p.meta.units.empty() ? "mm" : p.meta.units;
        j["meta"] = std::move(m);
    }

    // workarea
    if (p.workarea.size.x != 0 || p.workarea.size.y != 0) {
        json wa;
        wa["size"] = j_vec2(p.workarea.size);
        j["workarea"] = std::move(wa);
    }

    // materials
    if (!p.materials.empty()) {
        json mat = json::object();
        for (const auto& kv : p.materials) {
            const std::string& key = kv.first;
            const Material& m = kv.second;
            json mj;
            mj["color"] = m.color;
            mat[key] = std::move(mj);
        }
        j["materials"] = std::move(mat);
    }

    // parts
    {
        json arr = json::array();
        for (const Part& pr : p.parts) {
            json jp;
            if (!pr.id.empty()) jp["id"] = pr.id;
            jp["name"] = pr.name;
            if (!pr.source.empty()) jp["source"] = pr.source;
            if (!pr.material.empty()) jp["material"] = pr.material;
            jp["transform"] = j_transform(pr.transform);
            jp["visible"] = pr.visible;
            arr.push_back(std::move(jp));
        }
        j["parts"] = std::move(arr);
    }

    // animations
    if (!p.animations.empty()) {
        json A = json::object();
        for (const auto& kv : p.animations) {
            const std::string& key = kv.first;
            const Animation& a = kv.second;
            json ja;
            ja["component"] = a.component;
            ja["type"] = a.type;
            ja["from"] = j_vec3(a.from);
            ja["to"] = j_vec3(a.to);
            A[key] = std::move(ja);
        }
        j["animations"] = std::move(A);
    }

    std::string result;
    if (pretty)
        result = j.dump(2);
    else
        result = j.dump();

    // TODO: check if the file already exists
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    f.write(result.data(), static_cast<std::streamsize>(result.size()));

    return true;
}

void PrintProject(const Project& p) {
    std::ostringstream oss;

    oss << "Project (version " << p.version << ")\n";
    oss << "  Meta:\n";
    oss << "    Name   : " << p.meta.name << "\n";
    oss << "    Author : " << p.meta.author << "\n";
    oss << "    Units  : " << p.meta.units << "\n";

    oss << "  Workarea:\n";
    oss << "    Size: (" << p.workarea.size.x << ", " << p.workarea.size.y << ")\n";

    if (!p.materials.empty()) {
        oss << "  Materials:\n";
        for (const auto& kv : p.materials) {
            oss << "    " << kv.first << ": color=" << kv.second.color << "\n";
        }
    }

    if (!p.parts.empty()) {
        oss << "  Parts:\n";
        for (const auto& pr : p.parts) {
            oss << "    Part: " << pr.name << " (id=" << pr.id << ")\n";
            oss << "      source  : " << pr.source << "\n";
            oss << "      material: " << pr.material << "\n";
            oss << "      visible: " << pr.visible << "\n";
            oss << "      transform:\n";
            oss << "        translate = (" << pr.transform.translate.x << ", " << pr.transform.translate.y << ", "
                << pr.transform.translate.z << ")\n";
            oss << "        rotate    = (" << pr.transform.rotate.x << ", " << pr.transform.rotate.y << ", "
                << pr.transform.rotate.z << ")\n";
            oss << "        scale     = " << pr.transform.scale << "\n";
        }
    }

    if (!p.animations.empty()) {
        oss << "  Animations:\n";
        for (const auto& kv : p.animations) {
            const auto& a = kv.second;
            oss << "    " << kv.first << ":\n";
            oss << "      component: " << a.component << "\n";
            oss << "      type     : " << a.type << "\n";
            oss << "      from     : (" << a.from.x << ", " << a.from.y << ", " << a.from.z << ")\n";
            oss << "      to       : (" << a.to.x << ", " << a.to.y << ", " << a.to.z << ")\n";
        }
    }

    std::cout << oss.str() << std::endl;
}

}  // namespace io
