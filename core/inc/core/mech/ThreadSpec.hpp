#pragma once

namespace mech {

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

}  // namespace mech
