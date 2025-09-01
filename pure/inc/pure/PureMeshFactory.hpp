#pragma once
#include <memory>
#include <pure/PureMesh.hpp>

namespace pure {

class PureMeshFactory {
   public:
    // Simple cube from (0,0,0) to (size,size,height).
    static std::shared_ptr<PureMesh> CreateCube(float size, float height);
};
}  // namespace pure
