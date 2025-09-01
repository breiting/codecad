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

#include <Geom_Curve.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <geometry/Shape.hpp>
#include <string>

namespace mech {

/**
 * @brief Parameters for generating coarse 3D-print-friendly threads.
 *
 * Only `length`, `depth`, `turns` are conceptually required.
 * You must also provide the base diameter depending on external/internal usage.
 */
struct CoarseThreadParams {
    double length{10.0};  ///< Thread length along Z-axis (mm)
    double depth{1.0};    ///< Radial thread height from core to peak (mm)
    int turns{5};         ///< Number of complete helix revolutions (>=1)

    // Thread geometry parameters
    double flankAngleDeg{60.0};  ///< Included V-angle of the thread profile (degrees)
    double clearance{0.0};       ///< Global clearance for loose fits (mm)
    bool leftHand{false};        ///< True for left-hand thread, false for right-hand

    // Quality parameters
    int helixSegments{96};   ///< Points per revolution for helix smoothness
    double tolerance{1e-6};  ///< Geometric tolerance for operations
};

/**
 * @brief Thread validation result with error details
 */
struct ThreadValidation {
    bool isValid{true};
    std::string errorMessage;

    explicit operator bool() const {
        return isValid;
    }
};

/**
 * @brief Minimal coarse thread generator for 3D printing applications.
 *
 * This class generates simplified thread geometries optimized for 3D printing:
 * - External threads: Complete threaded rod (core cylinder + helical ridges)
 * - Internal threads: Cutter geometry to subtract from your part
 *
 * The thread profile is a simple isosceles triangle (V-shape) which is more
 * printable than precise ISO thread profiles.
 */
struct CoarseThread {
    /**
     * @brief Generate an external (male) coarse thread as a solid.
     *
     * Creates a complete threaded rod by fusing a core cylinder with helical ridges.
     * The core diameter is automatically calculated to provide the specified thread depth.
     *
     * @param outerDiameter  Nominal outer diameter of the threaded rod (mm)
     * @param threadParams   Thread geometry and quality parameters
     * @return geometry::ShapePtr  Complete threaded rod solid ready for 3D printing
     *
     * @throws std::invalid_argument if parameters are invalid
     * @throws std::runtime_error if geometry operations fail
     *
     * Geometry details:
     * - Effective outer diameter = outerDiameter - 2*clearance
     * - Core radius = (effective_outer_diameter/2) - depth
     * - Pitch radius = (effective_outer_diameter/2) - depth/2
     * - Thread ridges are fused with the core cylinder
     */
    static geometry::ShapePtr CreateExternalThread(double outerDiameter, const CoarseThreadParams& threadParams);

    /**
     * @brief Generate an internal (female) thread cutter.
     *
     * Creates a helical cutter geometry that you subtract from your part to create
     * internal threads. The cutter is designed to match external threads created
     * with the same parameters.
     *
     * @param boreDiameter   Diameter of the cylindrical hole in your part (mm)
     * @param threadParams   Thread geometry parameters (must match external thread)
     * @return geometry::ShapePtr  Cutter solid to subtract from your part
     *
     * @throws std::invalid_argument if parameters are invalid
     * @throws std::runtime_error if geometry operations fail
     *
     * Usage example:
     * @code
     * TopoDS_Shape nutBody = BRepPrimAPI_MakeCylinder(15, 10).Shape(); // nut body
     * auto cutter = CoarseThread::CreateInternalThread(12.0, params);
     * TopoDS_Shape threadedNut = BRepAlgoAPI_Cut(nutBody, cutter->getShape()).Shape();
     * @endcode
     *
     * Geometry details:
     * - Effective bore diameter = boreDiameter + 2*clearance
     * - Pitch radius = (effective_bore_diameter/2) + depth/2
     * - Cutter points inward to cut thread valleys into your part
     */
    static geometry::ShapePtr CreateInternalThread(double boreDiameter, const CoarseThreadParams& threadParams);

    /**
     * @brief Validate thread parameters before geometry creation.
     *
     * Checks parameter ranges and geometric compatibility to prevent runtime errors.
     *
     * @param outerOrBoreDiameter  The diameter to validate against
     * @param threadParams         Parameters to validate
     * @param isExternal          True for external thread validation
     * @return ThreadValidation    Validation result with error details
     */
    static ThreadValidation ValidateParameters(double outerOrBoreDiameter, const CoarseThreadParams& threadParams,
                                               bool isExternal = true);

    /**
     * @brief Calculate the pitch (distance between thread peaks) for given parameters.
     *
     * @param threadParams  Thread parameters
     * @return double       Pitch in millimeters (length / turns)
     */
    static double CalculatePitch(const CoarseThreadParams& threadParams);

   private:
    // Internal helper methods
    static Handle(Geom_Curve)
        CreateHelixCurve(double pitchRadius, double pitch, double length, bool leftHand, int segments);

    static TopoDS_Wire CreateTriangularProfile(double depth, double flankAngleDeg);

    static TopoDS_Wire TransformProfileToPosition(const TopoDS_Wire& profile, double pitchRadius,
                                                  bool pointingInward = false);

    static TopoDS_Shape CreateHelicalSweep(const TopoDS_Wire& helixPath, const TopoDS_Wire& profile);

    static double ClampToPositive(double value, double minValue = 1e-6);
};

}  // namespace mech
