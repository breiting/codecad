#pragma once
/**
 * @file CoarseThread.hpp
 * @brief Minimal coarse thread generator for 3D printing (external & internal).
 *
 * Concept:
 * - You specify: length, depth (radial height), turns (number of revolutions).
 * - For external threads you also give the outer diameter of the screw.
 * - For internal threads you give the bore (inner) diameter of the nut.
 * - Pitch = length / turns.
 * - We place a simple isosceles V-profile (default 60°) on the pitch radius and sweep along a helix.
 *
 * External thread:
 *   core radius  = outer_radius - depth  (simple core cylinder)
 *   thread ridges (swept volume) are FUSED to the core.
 *
 * Internal thread (Cutter):
 *   we return a CUTTER solid (the swept volume oriented inward).
 *   You subtract this from your solid to get the internal thread.
 *
 * All dimensions in millimeters.
 */

#include <TopoDS_Shape.hxx>
#include <geometry/Shape.hpp>

namespace mech {

/**
 * @brief Parameters for generating coarse 3D-print-friendly threads.
 *
 * Only `length`, `depth`, `turns` are conceptually required.
 * You must also provide the base diameter depending on external/internal usage.
 */
struct CoarseThreadParams {
    double length{10.0};  ///< Shaft length along Z
    double depth{1.0};    ///< Radial thread height (peak-to-core)
    int turns{5};         ///< Number of helix revolutions (>=1)

    // Printing & geometry refinements (optional)
    double flankAngleDeg{60.0};  ///< Included V-angle of the profile (default 60°)
    double clearance{0.0};       ///< Global clearance (mm), see notes below
    bool leftHand{false};        ///< Left-hand helix?
    int helixSegments{96};       ///< Helix sampling density (sweep smoothness)
};

/**
 * @brief Minimal thread generator (static helpers).
 *
 * Return values:
 * - External(): a complete threaded rod solid (core + ridges fused).
 * - InternalCutter(): a cutter solid you subtract from your part to create the internal thread.
 */
struct CoarseThread {
    /**
     * @brief Build an external (male) coarse thread as a solid.
     *
     * @param outerDiameter  Outer diameter of the screw (mm). Effective outer diameter will be (outerDiameter -
     * 2*clearance).
     * @param p              Thread parameters (length, depth, turns, ...)
     * @return TopoDS_Shape  Solid of the threaded rod.
     *
     * Notes:
     *  - Core cylinder radius = (outerDiameter - 2*clearance)/2 - depth (must stay > 0).
     *  - Ridges (swept volume) are fused to the core.
     */
    static geometry::ShapePtr External(double outerDiameter, const CoarseThreadParams& p);

    /**
     * @brief Build an internal (female) coarse thread cutter.
     *
     * @param boreDiameter   Cylinder bore (hole) diameter in your nut/part (mm).
     *                       Effective bore used for placement is (boreDiameter + 2*clearance).
     * @param p              Thread parameters (length, depth, turns, ...)
     * @return TopoDS_Shape  A CUTTER solid to subtract from your part to form the internal thread.
     *
     * Usage:
     *     TopoDS_Shape nutSolid = ...; // your base solid with a straight bore
     *     TopoDS_Shape cutter   = CoarseThread::InternalCutter(12.0, params);
     *     TopoDS_Shape nutWithThread = BRepAlgoAPI_Cut(nutSolid, cutter).Shape();
     */
    static geometry::ShapePtr InternalCutter(double boreDiameter, const CoarseThreadParams& p);
};

}  // namespace mech
