#include <array>
#include <core/Bounds.hpp>
#include <glm/gtc/matrix_transform.hpp>

AABB TransformAABB(const glm::mat4& M, const AABB& local) {
    AABB out;
    if (!local.valid) return out;

    const glm::vec3& mn = local.min;
    const glm::vec3& mx = local.max;
    const std::array<glm::vec3, 8> corners = {{
        {mn.x, mn.y, mn.z},
        {mx.x, mn.y, mn.z},
        {mn.x, mx.y, mn.z},
        {mx.x, mx.y, mn.z},
        {mn.x, mn.y, mx.z},
        {mx.x, mn.y, mx.z},
        {mn.x, mx.y, mx.z},
        {mx.x, mx.y, mx.z},
    }};
    for (const auto& c : corners) {
        glm::vec4 w = M * glm::vec4(c, 1.0f);
        out.Expand(glm::vec3(w));
    }
    return out;
}
