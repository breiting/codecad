#include <pure/PureMath.hpp>

namespace pure::math {

float ClosestPointsOnSegments(const Edge& s0, const Edge& s1, glm::vec3& c0, glm::vec3& c1) {
    // Based on "Real-Time Collision Detection" (Christer Ericson), robust clamps
    const glm::vec3 d1 = s0.b - s0.a;  // Direction vector of segment S1
    const glm::vec3 d2 = s1.b - s1.a;  // Direction vector of segment S2
    const glm::vec3 r = s0.a - s1.a;
    const float a = glm::dot(d1, d1);  // Squared length of segment S1
    const float e = glm::dot(d2, d2);  // Squared length of segment S2
    const float f = glm::dot(d2, r);

    float s, t;
    if (a <= 1e-12f && e <= 1e-12f) {
        // both degenerate
        c0 = s0.a;
        c1 = s1.a;
        return glm::length(c0 - c1);
    }
    if (a <= 1e-12f) {
        // First degenerate → project A onto S2
        s = 0.0f;
        t = glm::clamp(f / e, 0.0f, 1.0f);
    } else {
        const float c = glm::dot(d1, r);
        if (e <= 1e-12f) {
            // Second degenerate → project B onto S1
            t = 0.0f;
            s = glm::clamp(-c / a, 0.0f, 1.0f);
        } else {
            const float b = glm::dot(d1, d2);
            const float denom = a * e - b * b;
            if (denom != 0.0f)
                s = glm::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
            else
                s = 0.0f;  // parallel → fall back
            float tnom = (b * s + f);
            t = glm::clamp(tnom / e, 0.0f, 1.0f);

            // clamp s against 0/1 and recompute t (Ericson)
            if (denom != 0.0f) {
                if (t == 0.0f || t == 1.0f) {
                    s = glm::clamp((b * t - c) / a, 0.0f, 1.0f);
                    t = glm::clamp((b * s + f) / e, 0.0f, 1.0f);
                }
            }
        }
    }
    c0 = s0.a + d1 * s;
    c1 = s1.a + d2 * t;
    return glm::length(c0 - c1);
}

float ProjectedLength(glm::vec3 v, ConstraintAxis a) {
    switch (a) {
        case ConstraintAxis::X:
            return v.x;
        case ConstraintAxis::Y:
            return v.y;
        case ConstraintAxis::Z:
            return v.z;
        default:
            return glm::length(v);
    }
}

glm::vec3 Constrain(const glm::vec3& p0, const glm::vec3& p1, ConstraintAxis a) {
    if (a == ConstraintAxis::None) return p1;
    glm::vec3 d = p1 - p0;
    switch (a) {
        case ConstraintAxis::X:
            d = glm::vec3(d.x, 0, 0);
            break;
        case ConstraintAxis::Y:
            d = glm::vec3(0, d.y, 0);
            break;
        case ConstraintAxis::Z:
            d = glm::vec3(0, 0, d.z);
            break;
        default:
            break;
    }
    return p0 + d;
}

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
