#include <glm/gtc/constants.hpp>
#include <material/FlatShadedMaterial.hpp>
#include <scene/SphereNode.hpp>

SphereNode::SphereNode(float radius, int segments, int rings) {
    auto geo = std::make_unique<Mesh>();
    auto mat = std::make_shared<FlatShadedMaterial>();

    for (int y = 0; y <= rings; ++y) {
        float v = float(y) / rings;
        float theta = v * glm::pi<float>();

        for (int x = 0; x <= segments; ++x) {
            float u = float(x) / segments;
            float phi = u * glm::two_pi<float>();

            float px = radius * sin(theta) * cos(phi);
            float py = radius * cos(theta);
            float pz = radius * sin(theta) * sin(phi);

            glm::vec3 pos(px, py, pz);
            glm::vec3 normal = glm::normalize(pos);

            geo->AddVertex(Vertex(pos, normal, glm::vec3(0.0f)));
        }
    }

    for (int y = 0; y < rings; ++y) {
        for (int x = 0; x < segments; ++x) {
            int i0 = y * (segments + 1) + x;
            int i1 = i0 + segments + 1;

            geo->AddIndex(i0);
            geo->AddIndex(i1);
            geo->AddIndex(i0 + 1);

            geo->AddIndex(i1);
            geo->AddIndex(i1 + 1);
            geo->AddIndex(i0 + 1);
        }
    }
    m_Renderable = std::make_shared<Renderable>();
    m_Renderable->mesh = std::move(geo);
    m_Renderable->material = mat;
}
