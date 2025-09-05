#pragma once
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <vector>

#include "pure/PureBounds.hpp"

namespace pure {

class PureMesh;

struct PureMaterial {
    glm::vec3 baseColor{0.8f, 0.8f, 0.8f};
};

struct PurePart {
    std::string id;
    std::shared_ptr<PureMesh> mesh;
    glm::mat4 model{1.0f};
    PureMaterial material;
};

class PureScene {
   public:
    void AddPart(const std::string& id, const std::shared_ptr<PureMesh>& mesh, const glm::mat4& model,
                 const glm::vec3& color);
    void RemovePartById(const std::string& partId);
    void Clear();

    const std::vector<PurePart>& Parts() const {
        return m_Parts;
    }

    // Compute bounding box over all Parts (using Mesh bounds transformed)
    bool ComputeBounds(PureBounds& bounds) const;

   private:
    std::vector<PurePart> m_Parts;
};

}  // namespace pure
