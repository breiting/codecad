#pragma once

#include "geometry/Shape.hpp"

namespace mech {

/**
 * @brief Minimal printable involute spur gear (3D printable defaults).
 *
 * Parameters follow ISO/AGMA-ish conventions but stay pragmatic for FDM printing.
 * All lengths in mm, angles in degrees.
 */
struct GearSpec {
    int teeth = 20;             ///< z: tooth count (>= 8 recommended to avoid undercut)
    double module = 2.0;        ///< m: module (mm per tooth)
    double thickness = 6.0;     ///< gear width (Z)
    double bore = 5.0;          ///< center hole diameter
    double pressureDeg = 20;    ///< pressure angle α (°) (20° default)
    double backlash = 0.10;     ///< diametral backlash added at pitch (mm) for print fit
    double rootFillet = 0.30;   ///< tooth root fillet radius (mm), printer-friendly
    double tipRelief = 0.00;    ///< small tip chamfer/relief (mm), 0..0.2 usually enough
    double profileShift = 0.0;  ///< x: profile shift in module units (expert), default 0

    // Derived helpers (no need to fill by user):
    // pitch radius, addendum/dedendum, base radius etc. are computed internally
};

/**
 * @brief Utilities to compute mesh/fit relationships between two gears.
 */
struct GearPair {
    int z1, z2;
    double module;
    double centerDistance;  ///< nominal a = 0.5 * m * (z1 + z2), plus optional shift
};

/**
 * @brief Simple gear generator with pragmatic defaults for 3D printing.
 * Builds an involute spur gear from a single 2D tooth and revolves/arrays z times, then extrudes.
 */
class SimpleGear {
   public:
    /**
     * @brief Build a printable involute spur gear solid.
     * @param g  GearSpec with minimal fields (teeth, module, thickness, bore)
     * @return   geometry::ShapePtr (TopoDS_Shape) ready to triangulate/export
     */
    static geometry::ShapePtr MakeSpur(const GearSpec& g);

    /**
     * @brief Compute center distance for a pair (no profile shift): a = 0.5 * m * (z1 + z2).
     * @param z1   teeth gear 1
     * @param z2   teeth gear 2
     * @param m    module
     */
    static double CenterDistance(int z1, int z2, double m) {
        return 0.5 * m * (z1 + z2);
    }

    /**
     * @brief Produce a convenience GearPair description with center distance.
     */
    static GearPair MakePair(int z1, int z2, double module) {
        GearPair p{z1, z2, module, CenterDistance(z1, z2, module)};
        return p;
    }
};

}  // namespace mech
