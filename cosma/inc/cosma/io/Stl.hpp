#pragma once
#include <geometry/Mesh.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class StlReader {
   public:
    // Read ASCII/Binary STL as a Mesh.
    // Scale allows for converting units, e.g. if STL stores mm you can get meters by a value of 0.001
    bool ReadMesh(const std::string& fileName, Mesh& mesh, float scale) const;

    // Just read the data without generating a Mesh
    bool ReadRaw(const std::string& fileName, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals,
                 std::vector<unsigned int>& indices, float scale) const;
};
