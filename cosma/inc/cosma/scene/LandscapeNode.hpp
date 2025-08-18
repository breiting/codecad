#pragma once
#include <scene/ShapeNode.hpp>

class LandscapeNode : public ShapeNode {
   public:
    LandscapeNode(float xSizeMeters, float ySizeMeters, float maxHeightMeters, int gridResolution = 100);
};
