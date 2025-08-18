#pragma once
#include <glm/glm.hpp>
#include <vector>

struct CameraPathPoint {
    glm::vec3 position;
    glm::vec3 lookAtTarget;
};

class CameraPath {
   public:
    CameraPath();

    void AddPoint(const CameraPathPoint& point);
    void Update(float t);  // 0.0 - 1.0

    glm::vec3 GetCurrentPosition() const;
    glm::vec3 GetCurrentLookAt() const;

    void Clear();

   private:
    std::vector<CameraPathPoint> m_Points;
    float m_T;
};
