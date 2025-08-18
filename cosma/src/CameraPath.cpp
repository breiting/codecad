#include <core/CameraPath.hpp>
#include <glm/gtx/compatibility.hpp>  // for glm::lerp
#include <glm/gtx/normalize_dot.hpp>  // optional, for direction safety
#include <glm/gtx/quaternion.hpp>

CameraPath::CameraPath() : m_T(0.0f) {
}

void CameraPath::AddPoint(const CameraPathPoint& point) {
    m_Points.push_back(point);
}

void CameraPath::Update(float t) {
    m_T = glm::clamp(t, 0.0f, 1.0f);
}

void CameraPath::Clear() {
    m_Points.clear();
}

glm::vec3 CameraPath::GetCurrentPosition() const {
    if (m_Points.empty()) return glm::vec3(0.0f);

    if (m_Points.size() == 1) return m_Points.front().position;

    float segmentT = m_T * (m_Points.size() - 1);
    int index = static_cast<int>(segmentT);
    float localT = segmentT - index;

    if (index >= static_cast<int>(m_Points.size()) - 1) return m_Points.back().position;

    const glm::vec3& p0 = m_Points[index].position;
    const glm::vec3& p1 = m_Points[index + 1].position;

    return glm::lerp(p0, p1, localT);
}

glm::vec3 CameraPath::GetCurrentLookAt() const {
    if (m_Points.empty()) return glm::vec3(0.0f);

    if (m_Points.size() == 1) return m_Points.front().lookAtTarget;

    float segmentT = m_T * (m_Points.size() - 1);
    int index = static_cast<int>(segmentT);
    float localT = segmentT - index;

    if (index >= static_cast<int>(m_Points.size()) - 1) return m_Points.back().lookAtTarget;

    const glm::vec3& l0 = m_Points[index].lookAtTarget;
    const glm::vec3& l1 = m_Points[index + 1].lookAtTarget;

    return glm::lerp(l0, l1, localT);
}
