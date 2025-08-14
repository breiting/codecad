#pragma once
#include "geo/Shape.hpp"

namespace geo {

/// @brief Axis-aligned bounding box of a shape.
/// Units: mm; Empty box yields all zeros with isValid=false.
struct BBox {
    double minX{0}, minY{0}, minZ{0};
    double maxX{0}, maxY{0}, maxZ{0};
    bool isValid{false};

    double SizeX() const {
        return maxX - minX;
    }
    double SizeY() const {
        return maxY - minY;
    }
    double SizeZ() const {
        return maxZ - minZ;
    }
    double Cx() const {
        return 0.5 * (minX + maxX);
    }
    double Cy() const {
        return 0.5 * (minY + maxY);
    }
    double Cz() const {
        return 0.5 * (minZ + maxZ);
    }
};

/// @brief Compute AABB using OCCT (can use triangulation for speed).
/// @param useTriangulation When true, uses mesh if present; otherwise exact.
/// @return BBox with isValid=true on success.
BBox ComputeBBox(const ShapePtr& s, bool useTriangulation = true);

}  // namespace geo
