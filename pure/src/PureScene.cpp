#include <glm/gtx/transform.hpp>
#include <pure/PureMesh.hpp>
#include <pure/PureScene.hpp>

namespace pure {

void PureScene::AddPart(const std::shared_ptr<PureMesh>& mesh, const glm::mat4& model, const glm::vec3& color) {
    PurePart part;
    part.mesh = mesh;
    part.model = model;
    part.material.baseColor = color;
    m_Parts.push_back(std::move(part));
}

void PureScene::Clear() {
    m_Parts.clear();
}

bool PureScene::ComputeBounds(glm::vec3& outMin, glm::vec3& outMax) const {
    bool any = false;
    glm::vec3 minAll{0}, maxAll{0};

    auto TransformPoint = [](const glm::mat4& m, const glm::vec3& p) {
        glm::vec4 h = m * glm::vec4(p, 1.0f);
        return glm::vec3(h);
    };

    for (const auto& part : m_Parts) {
        if (!part.mesh) continue;
        PureAabb b = part.mesh->Bounds();
        if (!b.valid) continue;

        // 8 Eckpunkte transformieren
        glm::vec3 corners[8] = {
            {b.min.x, b.min.y, b.min.z}, {b.max.x, b.min.y, b.min.z}, {b.min.x, b.max.y, b.min.z},
            {b.max.x, b.max.y, b.min.z}, {b.min.x, b.min.y, b.max.z}, {b.max.x, b.min.y, b.max.z},
            {b.min.x, b.max.y, b.max.z}, {b.max.x, b.max.y, b.max.z},
        };
        glm::vec3 tmin, tmax;
        for (int i = 0; i < 8; ++i) {
            glm::vec3 tp = TransformPoint(part.model, corners[i]);
            if (i == 0) {
                tmin = tmax = tp;
            } else {
                tmin = glm::min(tmin, tp);
                tmax = glm::max(tmax, tp);
            }
        }
        if (!any) {
            minAll = tmin;
            maxAll = tmax;
            any = true;
        } else {
            minAll = glm::min(minAll, tmin);
            maxAll = glm::max(maxAll, tmax);
        }
    }

    if (any) {
        outMin = minAll;
        outMax = maxAll;
    }
    return any;
}

}  // namespace pure
