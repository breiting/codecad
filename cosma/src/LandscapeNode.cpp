#include <stb_perlin.h>

#include <geometry/Mesh.hpp>
#include <material/FlatShadedMaterial.hpp>
#include <scene/LandscapeNode.hpp>

LandscapeNode::LandscapeNode(float xSize, float ySize, float maxHeight, int gridRes) {
    auto geo = std::make_unique<Mesh>();
    auto mat = std::make_shared<FlatShadedMaterial>();

    const float dx = xSize / (gridRes - 1);
    const float dy = ySize / (gridRes - 1);
    const float scale = 0.2f;  // scales the frequency of the noise

    for (int y = 0; y < gridRes; ++y) {
        for (int x = 0; x < gridRes; ++x) {
            float px = x * dx - xSize / 2.0f;
            float py = y * dy - ySize / 2.0f;
            float height = stb_perlin_noise3(px * scale, py * scale, 0.0f, 0, 0, 0) * 0.5f + 0.5f;

            Vertex v(glm::vec3(px, py, height * maxHeight), glm::vec3(0, 1, 0), glm::vec3(0.0f));
            geo->AddVertex(v);
        }
    }

    for (int y = 0; y < gridRes - 1; ++y) {
        for (int x = 0; x < gridRes - 1; ++x) {
            int i0 = y * gridRes + x;
            int i1 = i0 + 1;
            int i2 = i0 + gridRes;
            int i3 = i2 + 1;

            geo->AddIndex(i0);
            geo->AddIndex(i2);
            geo->AddIndex(i1);
            geo->AddIndex(i1);
            geo->AddIndex(i2);
            geo->AddIndex(i3);
        }
    }

    geo->RecalculateNormals();

    m_Renderable = std::make_shared<Renderable>();
    m_Renderable->mesh = std::move(geo);
    m_Renderable->material = mat;
}
