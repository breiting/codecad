#pragma once
#include <Geom_Curve.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>

#include "geometry/Shape.hpp"

namespace mech {

/**
 * @brief Tip style for the V-thread profile.
 *
 * Sharp : ideal triangular crest (no truncation)
 * Cut   : truncated (flat) crest using a simple ratio of the radial depth,
 *         i.e. a straight cut (like a small chamfer) to improve printability.
 */
enum class ThreadTip { Sharp = 0, Cut = 1 };

/**
 * @brief Parameter set for a coarse (print-friendly) helical thread.
 *
 * All dimensions in millimeters. The thread runs along +Z.
 * The pitch P is derived as P = length / turns.
 *
 * Conventions
 * - External thread (male): major diameter = outer diameter (crest-to-crest).
 * - Internal thread (female-cutter): bore diameter ≈ minor diameter (root-to-root).
 */
struct CoarseThreadParams {
    double length = 10.0;         ///< Threaded length along +Z [mm]
    int turns = 4;                ///< Number of turns (>=1)
    double depth = 0.6;           ///< Radial depth (root-to-crest) [mm]
    double clearance = 0.0;       ///< Optional radial clearance added/subtracted [mm] (>=0)
    double flankAngleDeg = 60.0;  ///< Included angle of V-profile (typ. 60°)
    bool leftHand = false;        ///< Left-hand thread if true

    ThreadTip tip = ThreadTip::Sharp;  ///< Tip style of crest (Sharp/Cut)
    double tipFlatRatio = 0.10;        ///< For tip==Cut: fraction of depth to truncate (0..1)
};

/**
 * @brief Coarse thread construction helpers (external thread, internal cutter, bolt, nut).
 *
 * Coordinate system: Z is the thread axis, shapes extrude along +Z.
 */
class CoarseThread {
   public:
    /**
     * @brief Build an external (male) thread solid by sweeping a V-profile along a helix
     *        and fusing it with a core cylinder (major radius - depth).
     *
     * @param outerDiameter Major crest-to-crest diameter [mm].
     * @param p             Thread parameters (length, turns, depth, tip style, …).
     * @return geometry::ShapePtr positive solid (core + ridges).
     */
    static geometry::ShapePtr ThreadExternal(double outerDiameter, const CoarseThreadParams& p);

    /**
     * @brief Build an internal thread cutter (female) to subtract from a pre-drilled bore.
     *
     * The returned shape is a positive "cutter" (the helical groove volume).
     * To obtain a nut/hole with thread, you typically:
     *   1) make a hex/cyl base solid with a straight bore,
     *   2) subtract the cutter.
     *
     * @param boreDiameter  Straight bore diameter (≈ minor diameter) [mm].
     * @param p             Thread parameters (length, turns, depth, tip style, …).
     * @return geometry::ShapePtr positive solid (to subtract from your part).
     */
    static geometry::ShapePtr ThreadInternal(double boreDiameter, const CoarseThreadParams& p);

    /**
     * @brief Build a bolt: optional unthreaded shank + threaded section + hex head.
     *
     * Geometry:
     * - Shank + thread are along +Z, head sits below at z in [-headHeight, 0].
     * - If p.length < totalLength -> a straight shank (major radius) is placed before the threaded section.
     *
     * @param outerDiameter   Major (crest) diameter [mm].
     * @param totalLength     Total stem length (shank + thread) [mm] (head excluded).
     * @param headAcrossFlats Hex head across-flats [mm].
     * @param headHeight      Hex head height [mm].
     * @param p               Thread parameters (length=threaded length).
     * @return geometry::ShapePtr solid bolt.
     */
    static geometry::ShapePtr MakeBolt(double outerDiameter, double totalLength, double headAcrossFlats,
                                       double headHeight, const CoarseThreadParams& p);

    /**
     * @brief Build a hex nut and cut an internal thread through it.
     *
     * @param boreDiameter  Straight bore diameter before threading (≈ minor diameter) [mm].
     * @param thickness     Nut thickness (extrusion along +Z) [mm].
     * @param acrossFlats   Hex across-flats [mm].
     * @param p             Thread parameters (length should ≈ thickness).
     * @return geometry::ShapePtr solid nut with internal thread.
     */
    static geometry::ShapePtr MakeNut(double boreDiameter, double thickness, double acrossFlats,
                                      const CoarseThreadParams& p);

    static geometry::ShapePtr Test(const CoarseThreadParams& p);
};

}  // namespace mech
