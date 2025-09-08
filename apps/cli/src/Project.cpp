#include "Project.hpp"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

using json = nlohmann::json;

// -------- PARAM helpers ----------
std::optional<ParamValue> GetParam(const Project& p, const std::string& key) {
    auto it = p.params.find(key);
    if (it == p.params.end()) return std::nullopt;
    return it->second;
}
void SetParam(Project& p, const std::string& key, ParamValue v) {
    p.params[key] = std::move(v);
}
bool RemoveParam(Project& p, const std::string& key) {
    return p.params.erase(key) > 0;
}

static std::string readFile(const std::string& p) {
    std::ifstream f(p);
    if (!f) throw std::runtime_error("LoadProject: cannot open file: " + p);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
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

static json j_vec3(const glm::vec3& v) {
    return json::array({v.x, v.y, v.z});
}

static json j_transform(const PartTransform& t) {
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

static PartTransform getTransform(const json& jt) {
    PartTransform t{};
    if (jt.is_null()) return t;
    t.translate = getVec3(jt, "translate");
    t.rotate = getVec3(jt, "rotate");
    t.scale = jt["scale"].get<double>();
    return t;
}

void Project::Load(const std::string& path) {
    materials.clear();
    parts.clear();
    animations.clear();

    const auto txt = readFile(path);
    json j = json::parse(txt);

    version = j.value("version", 1);

    // meta
    if (j.contains("meta")) {
        const auto& m = j["meta"];
        meta.name = m.value("name", "");
        meta.author = m.value("author", "");
        meta.units = m.value("units", "mm");
    }

    // params
    if (j.contains("params") && j["params"].is_object()) {
        for (auto it = j["params"].begin(); it != j["params"].end(); ++it) {
            const auto& v = it.value();
            if (v.is_boolean()) {
                params[it.key()] = ParamValue::FromBool(v.get<bool>());
            } else if (v.is_number_float() || v.is_number_integer() || v.is_number_unsigned()) {
                params[it.key()] = ParamValue::FromNumber(v.get<double>());
            } else if (v.is_string()) {
                params[it.key()] = ParamValue::FromString(v.get<std::string>());
            } else {
                std::cerr << "Unknown parameter type for: " << it.key() << std::endl;
            }
        }
    }

    // materials
    if (j.contains("materials") && j["materials"].is_object()) {
        for (auto it = j["materials"].begin(); it != j["materials"].end(); ++it) {
            Material m{};
            m.color = it.value().value("color", "");
            materials[it.key()] = m;
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
            parts.emplace_back(std::move(pr));
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
            animations[it.key()] = std::move(anim);
        }
    }
}

bool Project::Save(const std::string& path, bool pretty) const {
    json j;

    // version
    j["version"] = version;

    // meta
    {
        json m;
        m["name"] = meta.name;
        m["author"] = meta.author;
        m["units"] = meta.units.empty() ? "mm" : meta.units;
        j["meta"] = std::move(m);
    }

    // params
    json jp;
    for (const auto& kv : params) {
        const auto& k = kv.first;
        const auto& v = kv.second;
        switch (v.type) {
            case ParamValue::Type::Number:
                jp[k] = v.number;
                break;
            case ParamValue::Type::Boolean:
                jp[k] = v.boolean;
                break;
            case ParamValue::Type::String:
                jp[k] = v.string;
                break;
        }
    }
    j["params"] = jp;

    // materials
    if (!materials.empty()) {
        json mat = json::object();
        for (const auto& kv : materials) {
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
        for (const Part& pr : parts) {
            json jp;
            if (!pr.id.empty()) {
                jp["id"] = pr.id;
            } else {
                // fallback use ID
                jp["id"] = pr.name;
            }
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
    if (!animations.empty()) {
        json A = json::object();
        for (const auto& kv : animations) {
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

void Project::Print() {
    std::ostringstream oss;

    oss << "Project (version " << version << ")\n";
    oss << "  Meta:\n";
    oss << "    Name   : " << meta.name << "\n";
    oss << "    Author : " << meta.author << "\n";
    oss << "    Units  : " << meta.units << "\n";

    if (!params.empty()) {
        oss << "  Params:\n";
        for (const auto& kv : params) {
            if (kv.second.type == ParamValue::Type::Boolean) {
                oss << "    " << kv.first << ": " << kv.second.boolean << "\n";
            } else if (kv.second.type == ParamValue::Type::Number) {
                oss << "    " << kv.first << ": " << kv.second.number << "\n";
            } else if (kv.second.type == ParamValue::Type::String) {
                oss << "    " << kv.first << ": " << kv.second.string << "\n";
            }
        }
    }

    if (!materials.empty()) {
        oss << "  Materials:\n";
        for (const auto& kv : materials) {
            oss << "    " << kv.first << ": color=" << kv.second.color << "\n";
        }
    }

    if (!parts.empty()) {
        oss << "  Parts:\n";
        for (const auto& pr : parts) {
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

    if (!animations.empty()) {
        oss << "  Animations:\n";
        for (const auto& kv : animations) {
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
