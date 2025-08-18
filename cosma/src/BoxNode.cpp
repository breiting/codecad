#include <glm/glm.hpp>
#include <material/FlatShadedMaterial.hpp>
#include <memory>
#include <scene/BoxNode.hpp>

#include "geometry/Vertex.hpp"

BoxNode::BoxNode(float xSize, float ySize, float zSize) {
    auto geo = std::make_unique<Mesh>();
    auto mat = std::make_shared<FlatShadedMaterial>();

    const glm::vec3 faceNormals[] = {
        {0, 0, 1},   // front
        {0, 1, 0},   // top
        {0, 0, -1},  // back
        {0, -1, 0},  // bottom
        {-1, 0, 0},  // left
        {1, 0, 0}    // right
    };

    const glm::vec3 faceVertices[6][4] = {// front
                                          {{-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}},
                                          // top
                                          {{-1, 1, 1}, {1, 1, 1}, {1, 1, -1}, {-1, 1, -1}},
                                          // back
                                          {{1, -1, -1}, {-1, -1, -1}, {-1, 1, -1}, {1, 1, -1}},
                                          // bottom
                                          {{-1, -1, -1}, {1, -1, -1}, {1, -1, 1}, {-1, -1, 1}},
                                          // left
                                          {{-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1}},
                                          // right
                                          {{1, -1, 1}, {1, -1, -1}, {1, 1, -1}, {1, 1, 1}}};

    glm::vec3 col = glm::vec3(0.0f);
    for (int i = 0; i < 6; ++i) {
        glm::vec3 normal = faceNormals[i];
        glm::vec3 v0 = faceVertices[i][0];
        glm::vec3 v1 = faceVertices[i][1];
        glm::vec3 v2 = faceVertices[i][2];
        glm::vec3 v3 = faceVertices[i][3];

        Vertex verts[6];
        glm::vec3 scale(xSize * 0.5f, ySize * 0.5f, zSize * 0.5f);

        verts[0] = Vertex(v0 * scale, normal, col);
        verts[1] = Vertex(v1 * scale, normal, col);
        verts[2] = Vertex(v2 * scale, normal, col);

        verts[3] = Vertex(v2 * scale, normal, col);
        verts[4] = Vertex(v3 * scale, normal, col);
        verts[5] = Vertex(v0 * scale, normal, col);

        for (int j = 0; j < 6; ++j) {
            geo->AddVertex(verts[j]);
            geo->AddIndex(static_cast<unsigned int>(geo->VertexCount() - 1));
        }
    }

    m_Renderable = std::make_shared<Renderable>();
    m_Renderable->mesh = std::move(geo);
    m_Renderable->material = mat;
}
