#pragma once
/**
 * @file Thread.hpp
 * @brief Robust metric (ISO) thread generation for 3D printing applications.
 *
 * This module provides high-quality thread generation suitable for both
 * mechanical simulation and 3D printing. Features include:
 *
 * - Accurate ISO 60° thread profiles
 * - Print-friendly geometry with configurable clearances
 * - Support for both external (screw) and internal (nut) threads
 * - Automatic pitch calculation for standard metric threads
 * - Lead-in chamfers and root fillets for better printing
 * - Left/right hand thread support
 *
 * The generated threads use proper helical geometry created by sweeping
 * a trapezoidal profile along a 3D helix path using OpenCASCADE.
 */

#include <TopoDS_Shape.hxx>

#include "geometry/Shape.hpp"

namespace mech {

/**
 * @brief ISO 60° thread profile parameters optimized for 3D printing.
 *
 * This structure defines the cross-sectional shape of the thread.
 * The fundamental triangle height H = (√3/2) × pitch forms the basis
 * for all calculations. Crest and root flats are specified as fractions
 * of H to ensure robust 3D printing while maintaining reasonable strength.
 */
struct ThreadProfile {
    /// Flank angle in degrees (60° for ISO metric threads)
    double flank_angle_deg = 60.0;

    /// Crest flat width as fraction of H (typically 1/8 for good strength)
    double crest_flat_factor = 0.125;  // 1/8

    /// External (male) root flat width as fraction of H
    double root_flat_ext_factor = 0.125;  // 1/8

    /// Internal (female) root flat width as fraction of H
    /// (larger to provide clearance and easier printing)
    double root_flat_int_factor = 0.25;  // 1/4

    /// Thread engagement depth as fraction of H (< 1.0 for printing clearance)
    double engagement_factor = 0.75;
};

/**
 * @brief Comprehensive thread generation options.
 *
 * These parameters control both the geometric accuracy and 3D printing
 * characteristics of the generated threads.
 */
struct ThreadOptions {
    /// Generate left-hand thread if true (default: right-hand)
    bool left_hand = false;

    /// Radial clearance for 3D printing (mm)
    /// - External threads: diameter reduced by 2×clearance
    /// - Internal threads: diameter increased by 2×clearance
    double print_clearance = 0.15;

    /// Lead-in chamfer length (mm). Creates tapered thread start for easier engagement.
    /// Set to 0 to disable.
    double lead_in_length = 1.0;

    /// Lead-out chamfer length (mm). Creates tapered thread end.
    /// Set to 0 to disable.
    double lead_out_length = 0.5;

    /// Thread profile parameters
    ThreadProfile profile = {};

    /// Number of segments for helix approximation (affects smoothness)
    /// Higher values = smoother curves but larger file sizes
    int helix_segments = 64;

    /// Number of cross-sections along thread length for better accuracy
    int cross_sections = 0;  // 0 = auto-calculate based on length/pitch ratio

    /// Minimum wall thickness for internal threads (mm)
    /// The generated nut will have at least this much material around the thread
    double min_wall_thickness = 2.0;
};

/**
 * @brief Standard metric thread specifications.
 *
 * Provides standard pitch values for common metric thread sizes
 * according to ISO 262 standard.
 */
class MetricThreadStandard {
   public:
    /**
     * @brief Get standard coarse pitch for metric thread diameter.
     *
     * Returns the standard coarse pitch (largest pitch) for a given
     * metric thread diameter according to ISO 262.
     *
     * @param nominal_diameter Thread diameter in mm (e.g., 6.0 for M6)
     * @return Standard coarse pitch in mm
     */
    static double getCoarsePitch(double nominal_diameter);

    /**
     * @brief Get standard fine pitch options for metric thread diameter.
     *
     * @param nominal_diameter Thread diameter in mm
     * @return Vector of available fine pitches in mm (may be empty)
     */
    static std::vector<double> getFinePitches(double nominal_diameter);

    /**
     * @brief Check if diameter/pitch combination is standard.
     *
     * @param nominal_diameter Thread diameter in mm
     * @param pitch Thread pitch in mm
     * @return True if this is a standard ISO 262 combination
     */
    static bool isStandardCombination(double nominal_diameter, double pitch);

    /**
     * @brief Get theoretical minor diameter for external thread.
     *
     * @param nominal_diameter Major diameter in mm
     * @param pitch Thread pitch in mm
     * @return Minor (root) diameter in mm
     */
    static double getMinorDiameter(double nominal_diameter, double pitch);

    /**
     * @brief Get theoretical pitch diameter.
     *
     * @param nominal_diameter Major diameter in mm
     * @param pitch Thread pitch in mm
     * @return Pitch diameter in mm
     */
    static double getPitchDiameter(double nominal_diameter, double pitch);
};

/**
 * @brief Generate a high-quality external metric thread (screw/bolt).
 *
 * Creates a complete threaded rod with accurate ISO 60° profile.
 * The thread is generated by sweeping a trapezoidal cross-section
 * along a helical path. The result is suitable for both mechanical
 * simulation and 3D printing.
 *
 * Key features:
 * - Accurate involute thread form with proper flank angles
 * - Print clearance compensation for reliable printing
 * - Optional lead-in/out chamfers for easy assembly
 * - Solid core for strength
 *
 * @param major_diameter Outside diameter of thread (mm)
 * @param pitch Thread pitch - distance between adjacent thread peaks (mm)
 * @param length Total threaded length (mm)
 * @param options Thread generation and printing parameters
 * @return Solid threaded rod ready for use or further boolean operations
 */
geometry::ShapePtr MakeExternalMetricThread(double major_diameter, double pitch, double length,
                                            const ThreadOptions& options = {});

/**
 * @brief Generate a high-quality internal metric thread (nut/tapped hole).
 *
 * Creates a threaded hole by cutting thread helixes from a solid cylinder.
 * The minor diameter of the hole matches the major diameter of the corresponding
 * external thread, with print clearance applied.
 *
 * Key features:
 * - Properly sized to accept matching external thread
 * - Print clearance for reliable 3D printed assemblies
 * - Sufficient wall thickness for strength
 * - Optional chamfers for easy thread engagement
 *
 * @param nominal_diameter Thread size (e.g., 6.0 for M6) - matches external major diameter
 * @param pitch Thread pitch (mm) - must match external thread
 * @param length Length of threaded section (mm)
 * @param outer_diameter Outer diameter of nut (mm). If 0, calculated automatically.
 * @param options Thread generation and printing parameters
 * @return Solid nut with internal thread, ready for use
 */
geometry::ShapePtr MakeInternalMetricThread(double nominal_diameter, double pitch, double length,
                                            double outer_diameter = 0.0, const ThreadOptions& options = {});

/**
 * @brief Create a standard metric bolt (external thread with head).
 *
 * Generates a complete bolt with hexagonal head and threaded shank.
 * Dimensions follow DIN 933/ISO 4017 standard where applicable.
 *
 * @param nominal_diameter Thread diameter (e.g., 6.0 for M6)
 * @param pitch Thread pitch (mm). Use 0 for standard coarse pitch.
 * @param thread_length Length of threaded portion (mm)
 * @param head_height Height of bolt head (mm). Use 0 for standard.
 * @param head_width Width across flats of hex head (mm). Use 0 for standard.
 * @param options Thread generation options
 * @return Complete bolt solid
 */
geometry::ShapePtr MakeMetricBolt(double nominal_diameter, double pitch = 0.0, double thread_length = 20.0,
                                  double head_height = 0.0, double head_width = 0.0, const ThreadOptions& options = {});

/**
 * @brief Create a standard metric nut (internal thread with hex exterior).
 *
 * Generates a hexagonal nut with internal thread matching metric standards.
 * Dimensions follow DIN 934/ISO 4032 where applicable.
 *
 * @param nominal_diameter Thread diameter (e.g., 6.0 for M6)
 * @param pitch Thread pitch (mm). Use 0 for standard coarse pitch.
 * @param nut_height Height of nut (mm). Use 0 for standard.
 * @param nut_width Width across flats (mm). Use 0 for standard.
 * @param options Thread generation options
 * @return Complete nut solid
 */
geometry::ShapePtr MakeMetricNut(double nominal_diameter, double pitch = 0.0, double nut_height = 0.0,
                                 double nut_width = 0.0, const ThreadOptions& options = {});

/**
 * @brief Calculate thread engagement length for given torque requirements.
 *
 * Provides guidance on minimum thread engagement for structural applications.
 * Based on material properties and loading conditions.
 *
 * @param nominal_diameter Thread diameter (mm)
 * @param material_strength Material tensile strength (MPa)
 * @param safety_factor Safety factor (typically 2.0-4.0)
 * @return Minimum engagement length (mm)
 */
double CalculateMinEngagementLength(double nominal_diameter,
                                    double material_strength = 400.0,  // Typical for PLA/PETG
                                    double safety_factor = 3.0);

/**
 * @brief Validate thread parameters for manufacturability.
 *
 * Checks if the specified thread parameters are reasonable for 3D printing
 * and will result in functional threads.
 *
 * @param major_diameter Thread major diameter (mm)
 * @param pitch Thread pitch (mm)
 * @param print_clearance Print clearance setting (mm)
 * @return Error message if invalid, empty string if OK
 */
std::string ValidateThreadParameters(double major_diameter, double pitch, double print_clearance);

}  // namespace mech
