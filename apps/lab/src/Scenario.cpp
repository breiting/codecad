#include "Scenario.hpp"

#include <sstream>

#include "ccad/io/Export.hpp"
#include "pure/PureMesh.hpp"

using namespace pure;
using namespace ccad;
using namespace ccad::geom;

static inline TriangulationParams GetTriangulateParams() {
    geom::TriangulationParams params;
    params.angularDeflectionDeg = 25.0;
    params.linearDeflection = 0.2;
    params.parallel = true;
    return params;
}

glm::vec3 Scenario::Hex(const std::string& hex) {
    unsigned r = 0, g = 0, b = 0;
    if (hex[0] == '#') std::sscanf(hex.c_str() + 1, "%02x%02x%02x", &r, &g, &b);
    return glm::vec3(r / 255.f, g / 255.f, b / 255.f);
}

void Scenario::SaveSTL(const std::string& fileName) {
    for (unsigned i = 0; i < m_Shapes.size(); i++) {
        std::ostringstream fn;
        fn << fileName << "_" << i << ".stl";
        io::SaveSTL(m_Shapes[i], fn.str(), GetTriangulateParams());
    }
}

std::shared_ptr<PureMesh> Scenario::ShapeToMesh(const Shape& shape) {
    TriMesh tri = Triangulate(shape, GetTriangulateParams());

    std::vector<PureVertex> verts;
    verts.reserve(tri.positions.size());
    for (auto& p : tri.positions) {
        verts.push_back({glm::vec3((float)p.x, (float)p.y, (float)p.z), glm::vec3(0, 0, 0)});
    }
    auto mesh = std::make_shared<PureMesh>();
    mesh->Upload(verts, tri.indices);
    return mesh;
}
