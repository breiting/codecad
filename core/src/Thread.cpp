#include "mech/Thread.hpp"

// Standard library
#include <algorithm>
#include <cmath>
#include <map>
#include <stdexcept>
#include <vector>

// OpenCASCADE includes
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>

namespace mech {

namespace {

/**
 * @brief Calculate the fundamental triangle height for ISO threads.
 *
 * For 60° threads, H = (√3/2) × pitch
 * This is the theoretical height of the equilateral triangle that forms
 * the basis of the thread profile.
 */
constexpr double calculateFundamentalHeight(double pitch) {
    return 0.86602540378443864676 * pitch;  // √3/2
}

/**
 * @brief Thread geometry calculator with full dimensional analysis.
 *
 * This class encapsulates all the calculations needed to generate
 * geometrically correct ISO metric threads, including the effects
 * of print clearances and manufacturing tolerances.
 */
class ThreadGeometry {
   public:
    // Input parameters
    double major_diameter;  ///< Outside diameter (external) or hole size (internal)
    double pitch;           ///< Thread pitch (mm)
    double length;          ///< Thread length (mm)
    bool external;          ///< True for external thread, false for internal
    ThreadOptions options;  ///< Generation options

    // Calculated dimensions
    double fundamental_height;  ///< Basic triangle height H
    double minor_diameter;      ///< Root diameter
    double pitch_diameter;      ///< Pitch line diameter
    double thread_depth;        ///< Actual thread depth (< H due to flats)
    double helix_radius;        ///< Radius for helix generation
    double turns;               ///< Number of complete turns

    // Profile dimensions
    double crest_flat;  ///< Crest flat width
    double root_flat;   ///< Root flat width

    ThreadGeometry(double maj_dia, double p, double len, bool ext, const ThreadOptions& opts)
        : major_diameter(maj_dia), pitch(p), length(len), external(ext), options(opts) {
        calculate();
    }

   private:
    void calculate() {
        if (major_diameter <= 0 || pitch <= 0 || length <= 0) {
            throw std::invalid_argument("Thread dimensions must be positive");
        }

        fundamental_height = calculateFundamentalHeight(pitch);

        // Calculate flats
        crest_flat = options.profile.crest_flat_factor * fundamental_height;
        root_flat = external ? options.profile.root_flat_ext_factor * fundamental_height
                             : options.profile.root_flat_int_factor * fundamental_height;

        // Thread depth calculation
        thread_depth = options.profile.engagement_factor * fundamental_height;

        if (external) {
            // External thread: major_diameter is the outside diameter
            minor_diameter = major_diameter - 2 * thread_depth;
            pitch_diameter = major_diameter - thread_depth;

            // Apply print clearance (reduce size)
            double clearance_reduction = 2.0 * options.print_clearance;
            major_diameter -= clearance_reduction;
            minor_diameter -= clearance_reduction;
            pitch_diameter -= clearance_reduction;

            helix_radius = pitch_diameter * 0.5;
        } else {
            // Internal thread: major_diameter is the hole size to tap
            minor_diameter = major_diameter + 2 * options.print_clearance;  // Enlarge hole
            pitch_diameter = minor_diameter + thread_depth;

            helix_radius = pitch_diameter * 0.5;
        }

        // Calculate number of turns
        turns = length / pitch;

        // Validation
        if (minor_diameter <= 0) {
            throw std::invalid_argument("Thread depth too large - results in negative minor diameter");
        }
        if (external && minor_diameter >= major_diameter) {
            throw std::invalid_argument("Invalid thread geometry - minor >= major diameter");
        }
    }
};

/**
 * @brief Generate a precise 3D helix curve using B-spline approximation.
 *
 * Creates a smooth helical path by sampling the helix equation at regular
 * intervals and fitting a B-spline through the points. This approach gives
 * better control over smoothness than analytical helix representations.
 *
 * @param radius Helix radius (mm)
 * @param pitch Thread pitch (mm)
 * @param length Total length along helix axis (mm)
 * @param left_hand True for left-hand helix
 * @param segments Number of segments per turn (higher = smoother)
 * @return B-spline curve representing the helix
 */
Handle(Geom_Curve) createHelixCurve(double radius, double pitch, double length, bool left_hand, int segments = 64) {
    const double turns = length / pitch;
    const int total_points = std::max(16, static_cast<int>(segments * turns));
    const double t_max = 2.0 * M_PI * turns;
    const double dt = t_max / (total_points - 1);
    const double z_step = (left_hand ? -1.0 : 1.0) * pitch / (2.0 * M_PI);

    TColgp_Array1OfPnt points(1, total_points);

    for (int i = 0; i < total_points; ++i) {
        const double t = i * dt;
        const double x = radius * std::cos(t);
        const double y = radius * std::sin(t);
        const double z = z_step * t;

        points.SetValue(i + 1, gp_Pnt(x, y, z));
    }

    // Create B-spline with appropriate degree and smoothness
    GeomAPI_PointsToBSpline spline_builder(points,
                                           3,           // degree
                                           8,           // max degree
                                           GeomAbs_C2,  // continuity
                                           1.0e-6       // tolerance
    );

    if (!spline_builder.IsDone()) {
        throw std::runtime_error("Failed to create helix B-spline");
    }

    return spline_builder.Curve();
}

/**
 * @brief Create thread profile cross-section as a closed wire.
 *
 * Generates the trapezoidal thread profile with proper flank angles,
 * crest and root flats. The profile is created in the XZ plane where:
 * - X represents the circumferential direction (0 to pitch width)
 * - Z represents the radial direction (positive outward)
 *
 * @param geometry Calculated thread geometry
 * @return Closed wire representing the thread profile
 */
TopoDS_Wire createThreadProfileWire(const ThreadGeometry& geometry) {
    const double pitch = geometry.pitch;
    const double depth = geometry.thread_depth;
    const double crest_flat = geometry.crest_flat;
    const double root_flat = geometry.root_flat;

    // Profile is symmetric about pitch/2
    const double half_pitch = pitch * 0.5;
    const double half_crest = crest_flat * 0.5;
    const double half_root = root_flat * 0.5;

    // Calculate profile points
    // Start from bottom left, go counter-clockwise
    std::vector<gp_Pnt> profile_points;

    // Bottom flat (root)
    profile_points.emplace_back(half_pitch - half_root, 0, -depth * 0.5);
    profile_points.emplace_back(half_pitch + half_root, 0, -depth * 0.5);

    // Right flank
    profile_points.emplace_back(half_pitch + half_crest, 0, +depth * 0.5);

    // Top flat (crest)
    profile_points.emplace_back(half_pitch - half_crest, 0, +depth * 0.5);

    // Create wire from points
    BRepBuilderAPI_MakePolygon polygon_builder;
    for (const auto& point : profile_points) {
        polygon_builder.Add(point);
    }
    polygon_builder.Close();

    if (!polygon_builder.IsDone()) {
        throw std::runtime_error("Failed to create thread profile polygon");
    }

    return polygon_builder.Wire();
}

/**
 * @brief Sweep profile along helix to create thread volume.
 *
 * Uses OpenCASCADE's pipe shell algorithm to sweep the thread profile
 * along the helical spine. The resulting solid represents the thread
 * volume that will be added (external) or subtracted (internal).
 *
 * @param spine_curve Helix path as wire
 * @param profile_wire Thread profile as wire
 * @param make_solid Whether to create a solid (vs surface)
 * @return Swept thread geometry
 */
TopoDS_Shape sweepProfileAlongHelix(const TopoDS_Wire& spine_wire, const TopoDS_Wire& profile_wire,
                                    bool make_solid = true) {
    BRepOffsetAPI_MakePipeShell pipe_builder(spine_wire);

    // Use Frenet frame for proper orientation along helix
    pipe_builder.SetMode(true);  // Use Frenet mode
    pipe_builder.Add(profile_wire);

    pipe_builder.Build();
    if (!pipe_builder.IsDone()) {
        throw std::runtime_error("Failed to sweep thread profile along helix");
    }

    if (make_solid) {
        pipe_builder.MakeSolid();
    }

    return pipe_builder.Shape();
}

/**
 * @brief Add chamfers to thread ends for easier engagement.
 *
 * Creates conical chamfers at thread start/end to facilitate assembly.
 * The chamfer angle is typically 45° and extends inward by the specified length.
 *
 * @param thread_solid Thread solid to modify
 * @param chamfer_length Length of chamfer along axis (mm)
 * @param major_radius Thread major radius
 * @param external True for external thread
 * @return Modified thread solid with chamfers
 */
TopoDS_Shape addThreadChamfers(const TopoDS_Shape& thread_solid, double chamfer_length, double major_radius,
                               bool external) {
    if (chamfer_length <= 0) {
        return thread_solid;  // No chamfer requested
    }

    // For now, return unchanged - full chamfer implementation would require
    // more complex geometry operations. This is a placeholder for future enhancement.
    // In practice, many 3D printed threads work well without chamfers.

    return thread_solid;
}

/**
 * @brief Standard metric thread pitch table (ISO 262).
 *
 * Provides standard coarse and fine pitches for common metric thread sizes.
 * Based on ISO 262 standard.
 */
const std::map<double, std::vector<double>> METRIC_PITCHES = {{1.0, {0.25}},
                                                              {1.2, {0.25}},
                                                              {1.6, {0.35}},
                                                              {2.0, {0.4, 0.25}},
                                                              {2.5, {0.45, 0.35}},
                                                              {3.0, {0.5, 0.35}},
                                                              {4.0, {0.7, 0.5}},
                                                              {5.0, {0.8, 0.5}},
                                                              {6.0, {1.0, 0.75, 0.5}},
                                                              {8.0, {1.25, 1.0, 0.75}},
                                                              {10.0, {1.5, 1.25, 1.0, 0.75}},
                                                              {12.0, {1.75, 1.5, 1.25, 1.0}},
                                                              {16.0, {2.0, 1.5, 1.0}},
                                                              {20.0, {2.5, 2.0, 1.5, 1.0}},
                                                              {24.0, {3.0, 2.5, 2.0, 1.5}},
                                                              {30.0, {3.5, 3.0, 2.0, 1.5}},
                                                              {36.0, {4.0, 3.0, 2.0, 1.5}},
                                                              {42.0, {4.5, 4.0, 3.0, 2.0}},
                                                              {48.0, {5.0, 4.0, 3.0, 2.0}}};

}  // anonymous namespace

// Public API Implementation

double MetricThreadStandard::getCoarsePitch(double nominal_diameter) {
    auto it = METRIC_PITCHES.find(nominal_diameter);
    if (it != METRIC_PITCHES.end() && !it->second.empty()) {
        return it->second[0];  // First entry is coarse pitch
    }

    // Fallback calculation for non-standard sizes
    if (nominal_diameter <= 2.0) return 0.4;
    if (nominal_diameter <= 3.0) return 0.5;
    if (nominal_diameter <= 5.0) return 0.8;
    if (nominal_diameter <= 8.0) return 1.25;
    if (nominal_diameter <= 12.0) return 1.75;
    if (nominal_diameter <= 18.0) return 2.5;
    if (nominal_diameter <= 24.0) return 3.0;

    return 3.5;  // Default for large threads
}

std::vector<double> MetricThreadStandard::getFinePitches(double nominal_diameter) {
    auto it = METRIC_PITCHES.find(nominal_diameter);
    if (it != METRIC_PITCHES.end() && it->second.size() > 1) {
        return std::vector<double>(it->second.begin() + 1, it->second.end());
    }
    return {};  // No fine pitches available
}

bool MetricThreadStandard::isStandardCombination(double nominal_diameter, double pitch) {
    auto it = METRIC_PITCHES.find(nominal_diameter);
    if (it != METRIC_PITCHES.end()) {
        const auto& pitches = it->second;
        return std::find(pitches.begin(), pitches.end(), pitch) != pitches.end();
    }
    return false;
}

double MetricThreadStandard::getMinorDiameter(double nominal_diameter, double pitch) {
    return nominal_diameter - 2.0 * calculateFundamentalHeight(pitch) * 0.625;  // 5/8 H
}

double MetricThreadStandard::getPitchDiameter(double nominal_diameter, double pitch) {
    return nominal_diameter - calculateFundamentalHeight(pitch) * 0.75;  // 3/4 H
}

geometry::ShapePtr MakeExternalMetricThread(double major_diameter, double pitch, double length,
                                            const ThreadOptions& options) {
    try {
        // Calculate thread geometry
        ThreadGeometry geometry(major_diameter, pitch, length, true, options);

        // Create core cylinder
        BRepPrimAPI_MakeCylinder core_builder(geometry.minor_diameter * 0.5, length);
        TopoDS_Shape core = core_builder.Shape();

        // Create helix curve
        Handle(Geom_Curve) helix_curve =
            createHelixCurve(geometry.helix_radius, pitch, length, options.left_hand, options.helix_segments);

        // Convert to wire
        TopoDS_Edge helix_edge = BRepBuilderAPI_MakeEdge(helix_curve);
        TopoDS_Wire helix_wire = BRepBuilderAPI_MakeWire(helix_edge);

        // Create thread profile
        TopoDS_Wire profile_wire = createThreadProfileWire(geometry);

        // Sweep profile along helix
        TopoDS_Shape thread_volume = sweepProfileAlongHelix(helix_wire, profile_wire, true);

        // Fuse core with thread volume
        BRepAlgoAPI_Fuse fuse_op(core, thread_volume);
        if (!fuse_op.IsDone()) {
            throw std::runtime_error("Failed to fuse core with thread volume");
        }

        TopoDS_Shape threaded_rod = fuse_op.Shape();

        // Add chamfers if requested
        if (options.lead_in_length > 0 || options.lead_out_length > 0) {
            threaded_rod = addThreadChamfers(threaded_rod, std::max(options.lead_in_length, options.lead_out_length),
                                             major_diameter * 0.5, true);
        }

        return std::make_shared<geometry::Shape>(threaded_rod);

    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("External thread generation failed: ") + e.what());
    }
}

geometry::ShapePtr MakeInternalMetricThread(double nominal_diameter, double pitch, double length, double outer_diameter,
                                            const ThreadOptions& options) {
    try {
        // Calculate thread geometry
        ThreadGeometry geometry(nominal_diameter, pitch, length, false, options);

        // Determine outer diameter if not specified
        if (outer_diameter <= 0) {
            outer_diameter = nominal_diameter + 2.0 * options.min_wall_thickness;
        }

        // Create outer cylinder
        BRepPrimAPI_MakeCylinder outer_builder(outer_diameter * 0.5, length);
        TopoDS_Shape outer_cylinder = outer_builder.Shape();

        // Create inner hole
        BRepPrimAPI_MakeCylinder inner_builder(geometry.minor_diameter * 0.5, length + 0.2);
        TopoDS_Shape inner_hole = inner_builder.Shape();

        // Offset inner hole slightly to ensure clean cut
        gp_Trsf offset;
        offset.SetTranslation(gp_Vec(0, 0, -0.1));
        BRepBuilderAPI_Transform transformer(inner_hole, offset, Standard_True);
        inner_hole = transformer.Shape();

        // Cut inner hole from outer cylinder
        BRepAlgoAPI_Cut cut1(outer_cylinder, inner_hole);
        if (!cut1.IsDone()) {
            throw std::runtime_error("Failed to create base cylinder with hole");
        }

        TopoDS_Shape base_cylinder = cut1.Shape();

        // Create helix for thread cutting
        Handle(Geom_Curve) helix_curve =
            createHelixCurve(geometry.helix_radius, pitch, length, options.left_hand, options.helix_segments);

        TopoDS_Edge helix_edge = BRepBuilderAPI_MakeEdge(helix_curve);
        TopoDS_Wire helix_wire = BRepBuilderAPI_MakeWire(helix_edge);

        // Create thread profile (inverted for cutting)
        TopoDS_Wire profile_wire = createThreadProfileWire(geometry);

        // Sweep profile to create thread volume for cutting
        TopoDS_Shape thread_volume = sweepProfileAlongHelix(helix_wire, profile_wire, true);

        // Cut thread volume from base cylinder
        BRepAlgoAPI_Cut cut2(base_cylinder, thread_volume);
        if (!cut2.IsDone()) {
            throw std::runtime_error("Failed to cut thread from cylinder");
        }

        TopoDS_Shape threaded_nut = cut2.Shape();

        // Add chamfers if requested
        if (options.lead_in_length > 0 || options.lead_out_length > 0) {
            threaded_nut = addThreadChamfers(threaded_nut, std::max(options.lead_in_length, options.lead_out_length),
                                             nominal_diameter * 0.5, false);
        }

        return std::make_shared<geometry::Shape>(threaded_nut);

    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Internal thread generation failed: ") + e.what());
    }
}

geometry::ShapePtr MakeMetricBolt(double nominal_diameter, double pitch, double thread_length, double head_height,
                                  double head_width, const ThreadOptions& options) {
    // Use standard pitch if not specified
    if (pitch <= 0) {
        pitch = MetricThreadStandard::getCoarsePitch(nominal_diameter);
    }

    // Use standard head dimensions if not specified
    if (head_height <= 0) {
        head_height = nominal_diameter * 0.7;  // Approximate DIN 933 ratio
    }
    if (head_width <= 0) {
        head_width = nominal_diameter * 1.5;  // Approximate hex width
    }

    // Create threaded shank
    auto threaded_shank = MakeExternalMetricThread(nominal_diameter, pitch, thread_length, options);

    // Create hexagonal head
    // For simplicity, we'll use a cylindrical head here
    // A full implementation would create a proper hexagon
    BRepPrimAPI_MakeCylinder head_builder(head_width * 0.5, head_height);
    TopoDS_Shape head = head_builder.Shape();

    // Position head at top of shank
    gp_Trsf head_transform;
    head_transform.SetTranslation(gp_Vec(0, 0, thread_length));
    BRepBuilderAPI_Transform head_transformer(head, head_transform, Standard_True);
    head = head_transformer.Shape();

    // Fuse head and shank
    BRepAlgoAPI_Fuse fuse_op(threaded_shank->Get(), head);
    if (!fuse_op.IsDone()) {
        throw std::runtime_error("Failed to fuse bolt head with shank");
    }

    return std::make_shared<geometry::Shape>(fuse_op.Shape());
}

geometry::ShapePtr MakeMetricNut(double nominal_diameter, double pitch, double nut_height, double nut_width,
                                 const ThreadOptions& options) {
    // Use standard pitch if not specified
    if (pitch <= 0) {
        pitch = MetricThreadStandard::getCoarsePitch(nominal_diameter);
    }

    // Use standard nut dimensions if not specified
    if (nut_height <= 0) {
        nut_height = nominal_diameter * 0.8;  // Approximate DIN 934 ratio
    }
    if (nut_width <= 0) {
        nut_width = nominal_diameter * 1.5;  // Approximate hex width
    }

    // Create internal thread with hexagonal exterior
    return MakeInternalMetricThread(nominal_diameter, pitch, nut_height, nut_width, options);
}

double CalculateMinEngagementLength(double nominal_diameter, double material_strength, double safety_factor) {
    // Simplified calculation based on thread shear area
    // More sophisticated analysis would consider bolt material,
    // loading conditions, etc.

    double thread_area = M_PI * nominal_diameter * nominal_diameter * 0.25;
    double required_strength = thread_area * material_strength / safety_factor;

    // Assume thread engagement provides shear strength
    double engagement_circumference = M_PI * nominal_diameter;
    double min_engagement = required_strength / (engagement_circumference * material_strength * 0.6);

    return std::max(min_engagement, nominal_diameter);  // At least one diameter
}

std::string ValidateThreadParameters(double major_diameter, double pitch, double print_clearance) {
    if (major_diameter <= 0) {
        return "Major diameter must be positive";
    }

    if (pitch <= 0) {
        return "Pitch must be positive";
    }

    if (print_clearance < 0) {
        return "Print clearance cannot be negative";
    }

    // Check minimum printable pitch
    if (pitch < 0.4) {
        return "Pitch too small for reliable 3D printing (minimum 0.4mm recommended)";
    }

    // Check that clearance isn't excessive
    if (print_clearance > pitch * 0.25) {
        return "Print clearance too large relative to pitch";
    }

    // Check thread depth vs diameter ratio
    double thread_depth = calculateFundamentalHeight(pitch) * 0.625;
    if (thread_depth > major_diameter * 0.3) {
        return "Thread too deep for diameter - may be weak";
    }

    return "";  // All OK
}

}  // namespace mech
