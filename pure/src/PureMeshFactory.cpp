#include <pure/PureMeshFactory.hpp>

namespace pure {

std::shared_ptr<PureMesh> PureMeshFactory::CreateCube(float size, float height) {
    const float s = size;
    const float h = height;
    glm::vec3 p[8] = {{0, 0, 0}, {s, 0, 0}, {s, s, 0}, {0, s, 0}, {0, 0, h}, {s, 0, h}, {s, s, h}, {0, s, h}};

    auto addTri = [&](std::vector<PureVertex>& v, std::vector<unsigned>& i, unsigned a, unsigned b, unsigned c,
                      const glm::vec3& normal) {
        size_t base = v.size();
        v.push_back({p[a], normal});
        v.push_back({p[b], normal});
        v.push_back({p[c], normal});
        i.push_back((unsigned)base + 0);
        i.push_back((unsigned)base + 1);
        i.push_back((unsigned)base + 2);
    };

    std::vector<PureVertex> vertices;
    std::vector<unsigned> indices;

    // Z-up: Flächen-Normale entsprechend
    addTri(vertices, indices, 0, 1, 2, {0, 0, -1});
    addTri(vertices, indices, 0, 2, 3, {0, 0, -1});  // bottom (z=0) to -Z
    addTri(vertices, indices, 4, 6, 5, {0, 0, 1});
    addTri(vertices, indices, 4, 7, 6, {0, 0, 1});  // top    (z=s) to +Z
    addTri(vertices, indices, 0, 4, 5, {0, -1, 0});
    addTri(vertices, indices, 0, 5, 1, {0, -1, 0});  // y=0    to -Y
    addTri(vertices, indices, 3, 2, 6, {0, 1, 0});
    addTri(vertices, indices, 3, 6, 7, {0, 1, 0});  // y=s    to +Y
    addTri(vertices, indices, 0, 3, 7, {-1, 0, 0});
    addTri(vertices, indices, 0, 7, 4, {-1, 0, 0});  // x=0    to -X
    addTri(vertices, indices, 1, 5, 6, {1, 0, 0});
    addTri(vertices, indices, 1, 6, 2, {1, 0, 0});  // x=s    to +X

    auto mesh = std::make_shared<PureMesh>();
    mesh->Upload(vertices, indices);
    return mesh;
}
}  // namespace pure
