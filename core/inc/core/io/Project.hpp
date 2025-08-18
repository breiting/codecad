#pragma once
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

struct Transform {
    glm::vec3 translate{0, 0, 0};
    glm::vec3 rotate{0, 0, 0};  // degrees
    glm::vec3 scale{1, 1, 1};
};

struct Bed {
    std::string type;      // "rect" | "circle" | ""
    glm::vec2 size{0, 0};  // for rect: w,h; for circle: w=diameter
    std::string origin;    // "center" | "corner" | ""
};

struct Viewer {
    std::string bg;  // "#RRGGBB" or ""
    bool grid{true};
    double explode{0.0};
};

struct Material {
    std::string color;    // "#RRGGBB"
    double density{0.0};  // optional
};

struct Part {
    std::string id;
    std::string name;
    std::string stl;       // path (optional)
    std::string source;    // lua file path (optional)
    std::string material;  // material key (optional)
    std::string color;     // hex color (optional)
    Transform transform{};
    glm::vec3 explode{0, 0, 0};
    struct {
        glm::vec2 xy{0, 0};
        double angle{0};
        bool has{false};
    } bed_place;
    std::vector<std::string> tags;
    // meta: arbitrary map of string->string for now
    std::unordered_map<std::string, std::string> meta;
};

struct Animation {
    // "animations": {
    //   "slide_plate": {
    //     "component": "top",
    //     "type": "translate",
    //     "from": [0,0,0],
    //     "to":   [200,0,0],
    //     "t": 0.0,               // 0..1, vom Viewer steuerbar (Taste/Slider)
    //     "easing": "smoothstep"
    //   }
    // }
};

struct BOMItem {
    std::string name;
    int qty{1};
    std::string material;
    std::string dim;
    std::string note;
};

struct Project {
    int version{1};
    struct {
        std::string name;
        std::string author;
        std::string units{"mm"};
    } meta;
    Viewer viewer{};
    Bed bed{};
    std::unordered_map<std::string, Material> materials;
    std::vector<Part> parts;
    std::vector<BOMItem> bom;
    std::vector<Animation> animations;
};

/// Load a Project JSON file into memory. Throws std::runtime_error on error.
Project LoadProject(const std::string& path);
