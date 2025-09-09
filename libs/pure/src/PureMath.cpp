#include <pure/PureMath.hpp>

namespace pure::math {

float DistancePointPoint(glm::vec3 a, glm::vec3 b) {
    return glm::length(b - a);
}

float DistancePointEdge(glm::vec3 p, const Edge& e) {
    glm::vec3 d = e.b - e.a;
    float len2 = glm::dot(d, d);
    if (len2 < 1e-12f) return glm::length(p - e.a);
    float t = glm::clamp(glm::dot(p - e.a, d) / len2, 0.0f, 1.0f);
    glm::vec3 c = e.a + t * d;
    return glm::length(p - c);
}

float DistanceEdgeEdgeSimple(const Edge& e1, const Edge& e2, float angleDegTol) {
    glm::vec3 v1 = e1.b - e1.a;
    float L1 = glm::length(v1);
    glm::vec3 v2 = e2.b - e2.a;
    float L2 = glm::length(v2);
    if (L1 < 1e-6f || L2 < 1e-6f) return glm::length(0.5f * (e1.a + e1.b) - 0.5f * (e2.a + e2.b));

    glm::vec3 d1 = v1 / L1, d2 = v2 / L2;
    float c = glm::clamp(glm::dot(d1, d2), -1.0f, 1.0f);
    float ang = glm::degrees(std::acos(std::abs(c)));
    if (ang <= angleDegTol) {
        glm::vec3 d = (c >= 0.0f) ? d1 : -d1;  // gemeinsame Richtung
        glm::vec3 m1 = 0.5f * (e1.a + e1.b), m2 = 0.5f * (e2.a + e2.b);
        glm::vec3 delta = m2 - m1;
        glm::vec3 perp = delta - glm::dot(delta, d) * d;
        return glm::length(perp);
    }
    // Fallback: einfach & stabil
    return glm::length(0.5f * (e1.a + e1.b) - 0.5f * (e2.a + e2.b));
}

}  // namespace pure::math
