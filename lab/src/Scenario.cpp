#include "Scenario.hpp"

#include "geometry/Triangulate.hpp"
#include "pure/PureMesh.hpp"

using namespace pure;

glm::vec3 Scenario::Hex(const std::string& hex) {
    unsigned r = 0, g = 0, b = 0;
    if (hex[0] == '#') std::sscanf(hex.c_str() + 1, "%02x%02x%02x", &r, &g, &b);
    return glm::vec3(r / 255.f, g / 255.f, b / 255.f);
}

std::shared_ptr<PureMesh> Scenario::ShapeToMesh(const TopoDS_Shape& shape) {
    geometry::TriMesh tri = geometry::TriangulateShape(shape, /*defl*/ 0.3, /*ang*/ 25.0, /*parallel*/ true);

    std::vector<PureVertex> verts;
    verts.reserve(tri.positions.size());
    for (auto& p : tri.positions) {
        verts.push_back({glm::vec3((float)p.x, (float)p.y, (float)p.z), glm::vec3(0, 0, 0)});
    }
    auto mesh = std::make_shared<PureMesh>();
    mesh->Upload(verts, tri.indices);
    return mesh;
}
