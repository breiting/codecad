#pragma once
/**
 * @file PipeTransition.hpp
 * @brief Build a smooth revolved transition between two pipes using analytic blends (tanh / logistic / smoothstep).
 *
 * The transition is modeled as a solid of revolution around the Z axis. We sample the inner and outer radii
 * as functions of z in [0, length], build a closed XZ-profile (outer curve up, inner curve back),
 * and revolve it (0..2π) to obtain a watertight solid.
 *
 * Coordinate system:
 *   - Z is the pipe axis (0 .. length)
 *   - The profile is constructed in the XZ plane (y = 0), with X = radius(z), Z = axial position
 */

#include <TopoDS_Shape.hxx>

namespace mech {

struct PipeBlendOpts {
    enum class Kind { Tanh, Logistic, Smoothstep };

    Kind kind = Kind::Tanh;  ///< Blend function
    double steepness = 6.0;  ///< Controls transition sharpness (for Tanh/Logistic)
    int samples = 128;       ///< Number of samples along Z (>= 8)
    double minWall = 0.2;    ///< Minimum inner/outer separation (mm)
    double epsilon = 1e-4;   ///< Numerical robustness padding in mm (profile closure/boolean health)
};

/**
 * @brief Parameters of the two pipes and axial distance.
 *
 * All radii and length are millimeters.
 * Precondition: rInX >= 0, rOutX > rInX. (Class enforces min wall.)
 */
struct PipeEnds {
    double rIn0 = 10.0;    ///< inner radius at z=0
    double rOut0 = 12.0;   ///< outer radius at z=0
    double rIn1 = 15.0;    ///< inner radius at z=L
    double rOut1 = 18.0;   ///< outer radius at z=L
    double length = 50.0;  ///< axial length L (mm)
};

/**
 * @class PipeTransition
 * @brief Builds a smooth, printable transition solid between two coaxial pipes.
 *
 * Usage:
 *   PipeEnds ends{ rIn0, rOut0, rIn1, rOut1, L };
 *   PipeBlendOpts opts; opts.kind = PipeBlendOpts::Kind::Tanh; opts.steepness = 8.0; ...
 *   TopoDS_Shape s = PipeTransition::Build(ends, opts);
 */
class PipeTransition {
   public:
    /// Build a solid of revolution for the smooth transition between pipe A (z=0) and pipe B (z=L).
    /// @throws std::invalid_argument for inconsistent radii/length; std::runtime_error on OCC build failures.
    static TopoDS_Shape Build(const PipeEnds& ends, const PipeBlendOpts& opts = PipeBlendOpts{});

   private:
    // ----- helpers (private, CamelCase for methods; m_CamelCase for members if needed) -----
    static double Blend(double t, const PipeBlendOpts& o);      // t in [0,1] -> s in [0,1]
    static void Sanitize(PipeEnds& e, const PipeBlendOpts& o);  // enforce wall & positive radii
};

}  // namespace mech
