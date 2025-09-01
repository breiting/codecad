#pragma once
/**
 * @file Thread.hpp
 * @brief Metric (ISO-like) thread generation helpers (print-friendly).
 *
 * High-level helpers to generate external (screw) and internal (nut) threads by
 * sweeping a 2D profile along a helical spine. Geometry is built with OpenCascade.
 *
 * The profile is an ISO-60°-style trapezoid with configurable crest/root flats.
 * For 3D printing, a radial XY compensation (clearance) can be applied:
 *   - external thread: radius is reduced by xy
 *   - internal thread: radius is enlarged by xy
 *
 * The defaults are robust for FDM printing and can be tuned via ThreadOpts.
 */

#include <TopoDS_Shape.hxx>

#include "geometry/Shape.hpp"

namespace mech {

/**
 * @brief Print-oriented ISO-like 60° thread profile parameters.
 *
 * The thread's fundamental triangle height is H = (sqrt(3)/2) * pitch.
 * Crest/root flats are expressed as fractions of H (e.g., 1/8 H).
 * This model is intentionally simplified for robust 3D printing.
 * You can later switch to exact ISO truncations if needed.
 */
struct ThreadProfile {
    /// Flank angle in degrees (60° for ISO metric).
    double flankAngleDeg = 60.0;

    /// Crest flat width as fraction of H (typically 1/8 H).
    double crestFlatK = 1.0 / 8.0;

    /// External (male) root flat width as fraction of H (≈ 1/8 H).
    double rootFlatExtK = 1.0 / 8.0;

    /// Internal (female) root flat width as fraction of H (≈ 1/4 H).
    double rootFlatIntK = 1.0 / 4.0;
};

/**
 * @brief Thread generation options (print-centric).
 */
struct ThreadOpts {
    /// Left-hand thread if true (default: right-hand).
    bool leftHand = false;

    /// Radial XY compensation for 3D printing (mm). External uses -xy, internal +xy.
    double xy = 0.15;

    /// Lead-in chamfer length along Z (mm). If <= 0, disabled.
    double leadIn = 1.5;

    /// Profile parameters (flats etc.).
    ThreadProfile profile{};

    /// Helix tessellation density (only affects visualization smoothness).
    int segments = 96;
};

/**
 * @brief Coarse ISO pitch lookup (very small table; extend as needed).
 *
 * @param dNominal Nominal diameter (e.g. 6.0 for M6).
 * @return Pitch in mm (e.g. 1.0 for M6).
 */
double IsoCoarsePitch(double dNominal);

/**
 * @brief Build a *print-friendly* external metric thread solid (male).
 *
 * @param dMajor  Major (outside) diameter in mm.
 * @param pitch   Thread pitch (mm).
 * @param length  Threaded length along Z (mm).
 * @param o       Options (clearance, handedness, etc.).
 * @return Solid TopoDS_Shape of the threaded rod (core + thread fused).
 */
geometry::ShapePtr MakeExternalMetricThread(double dMajor, double pitch, double length, const ThreadOpts& o);

/**
 * @brief Build a *print-friendly* internal metric thread solid (female).
 *
 * @param dNominal Nominal diameter (e.g. 6.0 for M6).
 * @param pitch    Thread pitch (mm).
 * @param height   Threaded height along Z (mm).
 * @param o        Options (clearance, handedness, etc.).
 * @return Solid TopoDS_Shape of the threaded bore (cylinder cut by thread volume).
 */
geometry::ShapePtr MakeInternalMetricThread(double dNominal, double pitch, double height, const ThreadOpts& o);

}  // namespace mech
