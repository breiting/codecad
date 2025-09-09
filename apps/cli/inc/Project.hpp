#pragma once
#include <glm/glm.hpp>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct Meta {
    std::string name;
    std::string author;
    std::string units{"mm"};
};

struct PartTransform {
    glm::vec3 translate{0, 0, 0};
    glm::vec3 rotate{0, 0, 0};  // degrees
    double scale;
};

struct Material {
    std::string color;  // hex color
};

struct Part {
    std::string id;
    std::string name;
    std::string source;    // lua file path
    std::string material;  // material key
    bool visible;
    PartTransform transform;
};

struct Animation {
    std::string component;
    std::string type;
    glm::vec3 from;
    glm::vec3 to;
};

// ---------- PARAMS ----------
struct ParamValue {
    enum class Type { Number, Boolean, String };
    Type type{Type::Number};
    double number{0.0};
    bool boolean{false};
    std::string string;

    static ParamValue FromNumber(double v) {
        ParamValue p;
        p.type = Type::Number;
        p.number = v;
        return p;
    }
    static ParamValue FromBool(bool v) {
        ParamValue p;
        p.type = Type::Boolean;
        p.boolean = v;
        return p;
    }
    static ParamValue FromString(std::string v) {
        ParamValue p;
        p.type = Type::String;
        p.string = std::move(v);
        return p;
    }
};

using ParamsMap = std::map<std::string, ParamValue>;

class Project {
   public:
    Project() = default;
    void Load(const std::string& path);
    bool Save(const std::string& path, bool pretty = true) const;
    void Print();

    int version{1};
    Meta meta;

    ParamsMap params;

    std::unordered_map<std::string, Material> materials;
    std::vector<Part> parts;
    std::unordered_map<std::string, Animation> animations;
};

// helpers:
std::optional<ParamValue> GetParam(const Project& p, const std::string& key);
void SetParam(Project& p, const std::string& key, ParamValue v);
bool RemoveParam(Project& p, const std::string& key);
