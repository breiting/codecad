#pragma once
#include "geometry/Shape.hpp"
#include "mech/ThreadSpec.hpp"

namespace mech {

/**
 * @brief Namespace for ISO 262 standard
 */
namespace iso {

/**
 * @brief Calculate the fundamental triangle height for ISO threads.
 *
 * For 60° threads, H = (√3/2) × pitch
 * This is the theoretical height of the equilateral triangle that forms
 * the basis of the thread profile.
 */
constexpr double CalculateFundamentalHeight(double pitch) {
    return 0.86602540378443864676 * pitch;  // √3/2
}

/**
 * @brief Standard metric thread specifications.
 *
 * Provides standard pitch values for common metric thread sizes
 * according to ISO 262 standard.
 */
class MetricStandard {
   public:
    /**
     * @brief Get standard coarse pitch for metric thread diameter.
     *
     * Returns the standard coarse pitch (largest pitch) for a given
     * metric thread diameter according to ISO 262.
     *
     * @param nominalDiameter Thread diameter in mm (e.g., 6.0 for M6)
     * @return Standard coarse pitch in mm
     */
    static double GetCoarsePitch(double nominalDiameter);

    /**
     * @brief Get standard fine pitch options for metric thread diameter.
     *
     * @param nominalDiameter Thread diameter in mm
     * @return Vector of available fine pitches in mm (may be empty)
     */
    static std::vector<double> GetFinePitches(double nominalDiameter);
};

}  // namespace iso

/**
 * @brief Operations to generate threaded geometry (external ridge volume, internal cutter),
 * and convenience builders for rod/nut bodies using a given ThreadSpec.
 *
 * Design idea:
 *  - ThreadExternalRod:  creates a SOLID (core rod + ridges) with thread length L_thread on a base rod length L_rod.
 *  - ThreadInternalCutter: creates the subtraction volume for an internal thread of length L_thread.
 *
 * Both external & internal must share the same ThreadSpec (pitch/depth/angle/handedness/clearance) to fit.
 */
class ThreadOps {
   public:
    /**
     * @brief Build a solid rod (cylinder) of length rodLength and apply an external thread of length threadLength,
     * starting at z=0 along +Z. Returns the THREADED SOLID (core fused with ridges).
     *
     * @param spec Shared thread definition (majorDiameter is used as outer diameter for the thread).
     * @param rodLength Total rod length (>= threadLength).
     * @param threadLength Length of threaded section (<= rodLength).
     */
    static geometry::ShapePtr ThreadExternalRod(const ThreadSpec& spec, double rodLength, double threadLength);

    /**
     * @brief Build an INTERNAL thread CUTTER volume. You typically subtract this from your housing body.
     *
     * @param spec Shared thread definition. majorDiameter is interpreted as the mating external major diameter.
     *             The cutter will be constructed around boreDiameter (your pre-hole) with pitch radius adjusted
     *             by spec.depth and spec.clearance so the pair mates.
     * @param threadLength Length of threaded section carved into the bore.
     * @return geometry::ShapePtr Cutter (solid) that you subtract from your body.
     */
    static geometry::ShapePtr ThreadInternalCutter(const ThreadSpec& spec, double threadLength,
                                                   double& boreHoleDiameter);
};

}  // namespace mech
