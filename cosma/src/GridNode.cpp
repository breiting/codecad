#include <material/FlatShadedMaterial.hpp>
#include <random>
#include <scene/GridNode.hpp>

GridNode::GridNode(float xSize, float ySize, int subdivisions, bool center, bool randomZ) {
    auto geo = std::make_unique<Mesh>();
    auto mat = std::make_shared<FlatShadedMaterial>();

    int stepsX = subdivisions;
    int stepsY = subdivisions;

    float stepX = xSize / static_cast<float>(stepsX);
    float stepY = ySize / static_cast<float>(stepsY);

    float startX = 0.0f;
    float startY = 0.0f;
    glm::vec3 col = {1.0, 1.0, 1.0};

    if (center) {
        startX = -xSize / 2.0f;
        startY = -ySize / 2.0f;
    }

    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(-0.1f, 0.1f);

    for (int y = 0; y < stepsY; ++y) {
        for (int x = 0; x < stepsX; ++x) {
            glm::vec3 v0 = glm::vec3(startX + x * stepX, startY + y * stepY, randomZ ? distribution(generator) : 0.0f);
            glm::vec3 v1 =
                glm::vec3(startX + (x + 1) * stepX, startY + y * stepY, randomZ ? distribution(generator) : 0.0f);
            glm::vec3 v2 =
                glm::vec3(startX + (x + 1) * stepX, startY + (y + 1) * stepY, randomZ ? distribution(generator) : 0.0f);
            glm::vec3 v3 =
                glm::vec3(startX + x * stepX, startY + (y + 1) * stepY, randomZ ? distribution(generator) : 0.0f);

            glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            int baseIndex = static_cast<int>(geo->GetVertices().size());
            geo->AddVertex(Vertex(v0, normal, col));
            geo->AddVertex(Vertex(v1, normal, col));
            geo->AddVertex(Vertex(v2, normal, col));
            geo->AddVertex(Vertex(v3, normal, col));

            geo->AddIndex(baseIndex + 0);
            geo->AddIndex(baseIndex + 1);
            geo->AddIndex(baseIndex + 2);

            geo->AddIndex(baseIndex + 0);
            geo->AddIndex(baseIndex + 2);
            geo->AddIndex(baseIndex + 3);
        }
    }
    m_Renderable = std::make_shared<Renderable>();
    m_Renderable->mesh = std::move(geo);
    m_Renderable->material = mat;
}
