#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <string>
#include <vector>

struct MeshData {
    std::vector<float> positions;       // x,y,z (size % 3 == 0)
    std::vector<float> normals;         // nx,ny,nz  (optional)
    std::vector<unsigned int> indices;  // triangles
};

struct PartVisual {
    std::string id;
    std::string name;
    MeshData mesh;
    glm::mat4 transform{1.0f};
    glm::vec3 color{0.9f, 0.85f, 0.6f};  // fallback
    bool visible{true};
};
