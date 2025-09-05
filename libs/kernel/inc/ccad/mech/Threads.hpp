#pragma once

#include <ccad/base/Shape.hpp>
#include <vector>

namespace ccad {
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
 * @brief Handedness (right-hand is the usual default).
 */
enum class Handedness { Right, Left };

/**
 * @brief Tip style for thread ends (visual + assembly aid).
 * - Sharp: keep the full height to the edge
 * - Cut: flat cut (simple chamfer) by ratio of depth
 */
enum class TipStyle { Sharp, Cut };

/**
 * @brief Unified thread specification shared by external and internal threads.
 *
 * All dimensions are in the same length units as your scene (e.g. mm).
 * You typically define this once (pitch, depth, flank, clearance, etc.)
 * and reuse it for both the male (rod/bolt) and female (nut/housing) parts.
 */
struct ThreadSpec {
    /// The requested bore hole diameter which would work with a smooth bolt (will be corrected by clearance)
    double fitDiameter = 20.0;

    /// Thread pitch (distance between ridges along Z).
    double pitch = 2.0;

    /// Radial thread height (ridge depth). For coarse 3D-print threads choose larger values for strength.
    double depth = 1.0;

    /// Flank angle (included) in degrees. 60° for metric-like triangular flanks; for coarse prints often fine too.
    double flankAngleDeg = 60.0;

    /// Clearance added/subtracted for print fit (positive opens the pair). Applied symmetrically by ops.
    double clearance = 0.0;

    /// Thread handedness (default Right).
    Handedness handedness = Handedness::Right;

    /// End treatment for Start/End of the threaded section.
    TipStyle tip = TipStyle::Cut;

    /// Ratio (0..1) of the thread depth used as simple chamfer height when TipStyle::Cut is active.
    double tipCutRatio = 0.5;

    /// Number of path segments per revolution used for B-spline sampling of the helix (quality/perf tradeoff).
    int segmentsPerTurn = 96;

    /// Validate and clamp non-sense values (simple guard rails).
    void Normalize() {
        if (pitch <= 0.0) pitch = 0.1;
        if (depth <= 0.0) depth = 0.1;
        if (flankAngleDeg < 10.0) flankAngleDeg = 10.0;
        if (flankAngleDeg > 120.0) flankAngleDeg = 120.0;
        if (segmentsPerTurn < 16) segmentsPerTurn = 16;
    }
};

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
    static Shape ThreadExternalRod(const ThreadSpec& spec, double rodLength, double threadLength,
                                   double& outerDiameter);

    /**
     * @brief Build an INTERNAL thread CUTTER volume. You typically subtract this from your housing body.
     *
     * @param spec Shared thread definition. majorDiameter is interpreted as the mating external major diameter.
     *             The cutter will be constructed around boreDiameter (your pre-hole) with pitch radius adjusted
     *             by spec.depth and spec.clearance so the pair mates.
     * @param threadLength Length of threaded section carved into the bore.
     * @return geometry::ShapePtr Cutter (solid) that you subtract from your body.
     */
    static Shape ThreadInternalCutter(const ThreadSpec& spec, double threadLength, double& boreHoleDiameter);
};

}  // namespace mech
}  // namespace ccad
