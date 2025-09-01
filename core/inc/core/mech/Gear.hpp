/// \file Gear.hpp
/// Geometry API for solid modeling (OCCT wrappers).
/// Involute gear generation optimized for 3D printing support.
#pragma once
#include "geometry/Shape.hpp"

namespace mech {

/**
 * @brief Parameters for gear generation, optimized for 3D printing
 */
struct GearParams {
    /// Number of teeth (minimum 6 for proper meshing)
    int teeth = 12;

    /// Module in mm (controls tooth size: pitch_diameter = teeth * module)
    double module = 2.0;

    /// Gear thickness/width in Z direction (mm)
    double thickness = 5.0;

    /// Center bore diameter (mm). Set to 0 for no bore.
    double bore_diameter = 0.0;

    /// Pressure angle in degrees (typically 20° for standard gears)
    double pressure_angle_deg = 20.0;

    /// Addendum coefficient (typically 1.0 for standard gears)
    double addendum_coeff = 1.0;

    /// Dedendum coefficient (typically 1.25 for standard gears)
    double dedendum_coeff = 1.25;

    /// Root fillet radius as fraction of module (0.3-0.4 typical)
    double root_fillet_factor = 0.35;

    /// Number of points for involute curve approximation
    int involute_samples = 20;

    /// Backlash compensation for 3D printing (reduces tooth thickness by this amount in mm)
    double backlash_compensation = 0.05;
};

/**
 * @brief Generate an involute spur gear optimized for 3D printing
 *
 * This function creates a standard involute spur gear with proper geometry
 * for mechanical applications. The gear is generated with:
 * - Accurate involute tooth profiles
 * - Proper root fillets for strength
 * - 3D printing optimizations (backlash compensation)
 * - Optional center bore
 *
 * @param params Gear generation parameters
 * @return ShapePtr to the generated gear solid
 */
geometry::ShapePtr MakeInvoluteGear(const GearParams& params);

/**
 * @brief Legacy interface for backward compatibility
 *
 * Creates a gear using the old parameter interface. New code should use
 * the GearParams version for better control and 3D printing optimization.
 *
 * @param z Number of teeth
 * @param m Module (mm)
 * @param th Thickness (mm)
 * @param bore Bore diameter (mm)
 * @param pressureDeg Pressure angle (degrees)
 * @return ShapePtr to the generated gear solid
 */
geometry::ShapePtr MakeInvoluteGear(int z, double m, double th, double bore, double pressureDeg = 20.0);

/**
 * @brief Calculate pitch circle diameter for a gear
 * @param teeth Number of teeth
 * @param module Module (mm)
 * @return Pitch circle diameter (mm)
 */
inline double GearPitchDiameter(int teeth, double module) {
    return teeth * module;
}

/**
 * @brief Calculate center distance between two meshing gears
 * @param teeth1 Number of teeth on first gear
 * @param teeth2 Number of teeth on second gear
 * @param module Module (must be same for both gears)
 * @return Center distance (mm)
 */
inline double GearCenterDistance(int teeth1, int teeth2, double module) {
    return 0.5 * module * (teeth1 + teeth2);
}

}  // namespace mech
