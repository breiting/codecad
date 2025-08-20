#include <geometry/Mesh.hpp>
#include <glm/glm.hpp>
#include <material/InfinityGridMaterial.hpp>
#include <memory>
#include <scene/InfinityGridNode.hpp>

InfinityGridNode::InfinityGridNode() {
    auto mat = std::make_shared<InfinityGridMaterial>();
    m_Renderable = std::make_shared<Renderable>();
    m_Renderable->material = mat;
}
