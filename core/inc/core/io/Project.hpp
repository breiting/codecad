#pragma once
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace io {

struct Transform {
    glm::vec3 translate{0, 0, 0};
    glm::vec3 rotate{0, 0, 0};  // degrees
    double scale;
};

struct WorkArea {
    glm::vec2 size{0, 0};
};

struct ProjectMaterial {
    std::string name;
    std::string color;  // hex color
};

struct Part {
    std::string id;
    std::string name;
    std::string source;    // lua file path (optional)
    std::string material;  // material key (optional)
    Transform transform;
};

struct Animation {
    std::string component;
    std::string type;
    glm::vec3 from;
    glm::vec3 to;
};

struct Project {
    int version{1};
    struct {
        std::string name;
        std::string author;
        std::string units{"mm"};
    } meta;
    WorkArea workarea;
    std::unordered_map<std::string, ProjectMaterial> materials;
    std::vector<Part> parts;
    std::unordered_map<std::string, Animation> animations;
};

/// Load a Project JSON file into memory. Throws std::runtime_error on error.
Project LoadProject(const std::string& path);

/// Save a Project JSON file. Returns true on success
bool SaveProject(const Project& p, const std::string& path, bool pretty = true);

/// Prints the project to stdout
void PrintProject(const Project& p);

}  // namespace io
