#include <cstdint>
#include <cstring>
#include <fstream>
#include <io/Stl.hpp>
#include <sstream>

struct StlTriangle {
    float nx, ny, nz;
    float ax, ay, az;
    float bx, by, bz;
    float cx, cy, cz;
};

static bool StlCheckAscii(std::ifstream& f) {
    char header[6] = {0};
    f.read(header, 5);
    f.clear();
    f.seekg(0, std::ios::beg);
    return std::strncmp(header, "solid", 5) == 0;
}

static bool LoadBinarySTL(std::ifstream& f, std::vector<StlTriangle>& out) {
    f.seekg(80, std::ios::beg);
    uint32_t triCount = 0;
    if (!f.read(reinterpret_cast<char*>(&triCount), 4)) return false;
    out.resize(triCount);
    for (uint32_t i = 0; i < triCount; ++i) {
        if (!f.read(reinterpret_cast<char*>(&out[i].nx), 12)) return false;
        if (!f.read(reinterpret_cast<char*>(&out[i].ax), 12)) return false;
        if (!f.read(reinterpret_cast<char*>(&out[i].bx), 12)) return false;
        if (!f.read(reinterpret_cast<char*>(&out[i].cx), 12)) return false;
        uint16_t attr = 0;
        if (!f.read(reinterpret_cast<char*>(&attr), 2)) return false;
        (void)attr;
    }
    return true;
}

static bool LoadAsciiSTL(std::ifstream& f, std::vector<StlTriangle>& out) {
    f.clear();
    f.seekg(0, std::ios::beg);
    std::string line;
    StlTriangle cur{};
    int vcount = 0;
    while (std::getline(f, line)) {
        std::istringstream ss(line);
        std::string w;
        ss >> w;
        if (w == "facet") {
            std::string junk;
            ss >> junk;  // "normal"
            ss >> cur.nx >> cur.ny >> cur.nz;
            vcount = 0;
        } else if (w == "vertex") {
            float x, y, z;
            ss >> x >> y >> z;
            if (vcount == 0) {
                cur.ax = x;
                cur.ay = y;
                cur.az = z;
            } else if (vcount == 1) {
                cur.bx = x;
                cur.by = y;
                cur.bz = z;
            } else if (vcount == 2) {
                cur.cx = x;
                cur.cy = y;
                cur.cz = z;
            }
            vcount++;
        } else if (w == "endfacet") {
            if (vcount == 3) out.push_back(cur);
            vcount = 0;
        }
    }
    return !out.empty();
}

bool StlReader::ReadRaw(const std::string& fileName, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals,
                        std::vector<unsigned int>& indices, float scale = 1.0) const {
    std::ifstream f(fileName, std::ios::binary);
    if (!f) return false;

    std::vector<StlTriangle> triangles;
    bool ok = StlCheckAscii(f) ? LoadAsciiSTL(f, triangles) : LoadBinarySTL(f, triangles);
    if (!ok) return false;

    vertices.clear();
    normals.clear();
    indices.clear();
    vertices.reserve(triangles.size() * 3);
    normals.reserve(triangles.size());
    indices.reserve(triangles.size() * 3);

    for (const auto& t : triangles) {
        const unsigned int base = static_cast<unsigned int>(vertices.size());
        vertices.emplace_back(t.ax * scale, t.ay * scale, t.az * scale);
        vertices.emplace_back(t.bx * scale, t.by * scale, t.bz * scale);
        vertices.emplace_back(t.cx * scale, t.cy * scale, t.cz * scale);
        normals.emplace_back(t.nx, t.ny, t.nz);
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
    }
    return true;
}

bool StlReader::ReadMesh(const std::string& fileName, Mesh& mesh, float scale = 1.0) const {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    if (!ReadRaw(fileName, vertices, normals, indices, scale)) return false;

    for (size_t i = 0, j = 0; i + 2 < vertices.size(); i += 3, j++) {
        mesh.AddVertex(Vertex(vertices[i], normals[j]));
        mesh.AddVertex(Vertex(vertices[i + 1], normals[j]));
        mesh.AddVertex(Vertex(vertices[i + 2], normals[j]));
    }

    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        mesh.AddTriangle(indices[i], indices[i + 1], indices[i + 2]);
    }

    // Optional: Recalculate normals
    mesh.RecalculateNormals();
    return true;
}
