#pragma once
#include <ccad/base/Math.hpp>
#include <ccad/base/Shape.hpp>
#include <glm/glm.hpp>

namespace ccad::geom {

/** Type of mid-surface height law along two in-plane directions. */
enum class BendLaw { Paraboloid, Sine };

/** Specification for a doubly-curved plate (mid-surface + thickness). */
struct CurvedPlateSpec {
    double sizeX{100.0};        ///< Plate extent in X (mm)
    double sizeY{100.0};        ///< Plate extent in Y (mm)
    glm::dvec2 dirU{1.0, 0.0};  ///< In-plane bend direction U (normalized internally)
    glm::dvec2 dirV{0.0, 1.0};  ///< In-plane bend direction V (normalized internally)
    double kU{0.0};             ///< Curvature/strength along U (1/mm for paraboloid, cycles/mm for sine)
    double kV{0.0};             ///< Curvature/strength along V
    BendLaw law{BendLaw::Paraboloid};
    double thickness{2.0};  ///< Plate thickness (mm)
    int nu{16};             ///< Samples along X
    int nv{16};             ///< Samples along Y
};

/**
 * @brief Build a doubly-curved plate as a closed solid.
 *
 * - Mid-surface is a BSpline from sampled points.
 * - Top/Bottom are true normal offsets (Geom_OffsetSurface).
 * - Side walls are ruled faces between corresponding edges.
 */
Shape CurvedPlate(const CurvedPlateSpec& spec);

}  // namespace ccad::geom
