#pragma once
#include <ccad/base/Shape.hpp>
#include <ccad/select/EdgeSelector.hpp>

namespace ccad {
namespace feature {

enum class ChamferRadialType { Undefined, External, Internal };

struct ChamferRadialSpec {
    ChamferRadialType type = ChamferRadialType::Undefined;
    double angleDeg = 45;  ///< Angle from XY plane towards Z
    double length = 2;     ///< Length of the chamfer (e.g. 2mm)
};

//
/// \brief Apply a chamfer on all edges with the given distance
Shape ChamferAll(const Shape& s, double distanceMm);

/**
 * @brief Apply a straight chamfer (bevel) to selected edges.
 *
 * Works for solid shapes; for planar profiles (WIRE) it will first
 * create a face from the wire and chamfer edges on that face.
 *
 * @param s          Input shape
 * @param edges      Edge set from EdgeSelector::collect()
 * @param distanceMm Symmetric chamfer distance (D1 == D2)
 * @return Shape     New shape with chamfers applied (or original on failure)
 */
Shape Chamfer(const Shape& s, const select::EdgeSet& edges, double distanceMm);

/// \brief Generates a radial chamfer cutter for rods (external) or holes (internal)
Shape ChamferCutterRadial(double diameter, const ChamferRadialSpec& spec);

}  // namespace feature
}  // namespace ccad
