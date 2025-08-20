#include <glm/glm.hpp>
#include <material/LineSetMaterial.hpp>
#include <memory>
#include <scene/AxesNode.hpp>

#include "geometry/LineSet.hpp"

AxesNode::AxesNode() {
    auto geo = std::make_unique<LineSet>();
    auto mat = std::make_shared<LineSetMaterial>();
    // X axis (red)
    geo->AddVertex({glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(1, 0, 0)});
    geo->AddVertex({glm::vec3(1, 0, 0), glm::vec3(1, 0, 0), glm::vec3(1, 0, 0)});

    // Y axis (green)
    geo->AddVertex({glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0)});
    geo->AddVertex({glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0)});

    // Z axis (blue)
    geo->AddVertex({glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1)});
    geo->AddVertex({glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1)});

    m_Renderable = std::make_shared<Renderable>();
    m_Renderable->lines = std::move(geo);
    m_Renderable->material = mat;
}
