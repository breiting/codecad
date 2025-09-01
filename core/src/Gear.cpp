#include "mech/Gear.hpp"

// Standard library
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

// OpenCASCADE includes
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeSegment.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Circle.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <gp_Circ.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>

namespace mech {

namespace {

/**
 * @brief Complete geometric parameters for an involute gear
 *
 * This structure contains all the calculated dimensions needed to generate
 * a proper involute spur gear according to standard gear theory.
 */
struct GearGeometry {
    // Input parameters
    double module;              ///< Module (mm)
    double pressure_angle_rad;  ///< Pressure angle (radians)
    int teeth;                  ///< Number of teeth
    double thickness;           ///< Gear thickness (mm)

    // Calculated radii
    double pitch_radius;     ///< Pitch circle radius (mm)
    double base_radius;      ///< Base circle radius (mm)
    double addendum_radius;  ///< Addendum (tip) circle radius (mm)
    double dedendum_radius;  ///< Dedendum (root) circle radius (mm)

    // Tooth geometry
    double tooth_angle;            ///< Angular pitch per tooth (radians)
    double base_tooth_thickness;   ///< Tooth thickness at base circle (radians)
    double pitch_tooth_thickness;  ///< Tooth thickness at pitch circle (radians)

    // Manufacturing adjustments
    double backlash_angle;      ///< Angular backlash compensation (radians)
    double root_fillet_radius;  ///< Root fillet radius (mm)

    /**
     * @brief Calculate all gear geometry from basic parameters
     */
    static GearGeometry calculate(const GearParams& params) {
        GearGeometry geom = {};

        // Validate input parameters
        if (params.teeth < 6) {
            throw std::invalid_argument("Gear must have at least 6 teeth");
        }
        if (params.module <= 0) {
            throw std::invalid_argument("Module must be positive");
        }
        if (params.thickness <= 0) {
            throw std::invalid_argument("Thickness must be positive");
        }

        // Basic parameters
        geom.module = params.module;
        geom.pressure_angle_rad = params.pressure_angle_deg * M_PI / 180.0;
        geom.teeth = params.teeth;
        geom.thickness = params.thickness;

        // Standard gear calculations
        geom.pitch_radius = 0.5 * params.teeth * params.module;
        geom.base_radius = geom.pitch_radius * std::cos(geom.pressure_angle_rad);
        geom.addendum_radius = geom.pitch_radius + params.addendum_coeff * params.module;
        geom.dedendum_radius = geom.pitch_radius - params.dedendum_coeff * params.module;

        // Ensure dedendum doesn't go negative or too small
        double min_dedendum = std::max(0.1 * params.module, geom.base_radius * 0.8);
        geom.dedendum_radius = std::max(geom.dedendum_radius, min_dedendum);

        // Tooth spacing and thickness
        geom.tooth_angle = 2.0 * M_PI / params.teeth;
        geom.pitch_tooth_thickness = M_PI * params.module;  // Circular pitch at pitch circle

        // Convert backlash compensation to angular
        geom.backlash_angle = params.backlash_compensation / geom.pitch_radius;

        // Root fillet
        geom.root_fillet_radius = params.root_fillet_factor * params.module;

        return geom;
    }
};

/**
 * @brief Generate points along an involute curve
 *
 * The involute of a circle is the curve traced by a point on a taut string
 * as it unwinds from the circle. This forms the working profile of gear teeth.
 *
 * Parametric equations for involute:
 * x = r * (cos(t) + t * sin(t))
 * y = r * (sin(t) - t * cos(t))
 *
 * where r is the base circle radius and t is the parameter.
 *
 * @param base_radius Base circle radius
 * @param start_radius Starting radius for involute
 * @param end_radius Ending radius for involute
 * @param samples Number of points to generate
 * @param mirror_y If true, mirror the curve about x-axis
 * @return Vector of points on the involute curve
 */
std::vector<gp_Pnt> generateInvolutePoints(double base_radius, double start_radius, double end_radius, int samples,
                                           bool mirror_y = false) {
    std::vector<gp_Pnt> points;
    points.reserve(samples + 1);

    if (start_radius < base_radius) start_radius = base_radius;
    if (end_radius < start_radius) end_radius = start_radius;

    // Calculate parameter range
    double t_start = (start_radius <= base_radius)
                         ? 0.0
                         : std::sqrt((start_radius * start_radius) / (base_radius * base_radius) - 1.0);
    double t_end = std::sqrt((end_radius * end_radius) / (base_radius * base_radius) - 1.0);

    for (int i = 0; i <= samples; ++i) {
        double t = t_start + (t_end - t_start) * double(i) / samples;

        double x = base_radius * (std::cos(t) + t * std::sin(t));
        double y = base_radius * (std::sin(t) - t * std::cos(t));

        if (mirror_y) y = -y;

        points.emplace_back(x, y, 0.0);
    }

    return points;
}

/**
 * @brief Create a B-spline curve from a set of points
 *
 * @param points Vector of 3D points
 * @return Handle to B-spline curve
 */
Handle(Geom_BSplineCurve) createBSplineFromPoints(const std::vector<gp_Pnt>& points) {
    if (points.size() < 2) {
        throw std::invalid_argument("Need at least 2 points for B-spline");
    }

    TColgp_Array1OfPnt pointArray(1, static_cast<Standard_Integer>(points.size()));
    for (size_t i = 0; i < points.size(); ++i) {
        pointArray.SetValue(static_cast<Standard_Integer>(i + 1), points[i]);
    }

    GeomAPI_PointsToBSpline splineBuilder(pointArray, 3, 8, GeomAbs_C2, 1.0e-6);
    if (!splineBuilder.IsDone()) {
        throw std::runtime_error("Failed to create B-spline curve");
    }

    return splineBuilder.Curve();
}

/**
 * @brief Create a single gear tooth profile as a closed wire
 *
 * The tooth profile consists of:
 * 1. Root circle arc (between teeth)
 * 2. Right involute curve (pressure side)
 * 3. Tip circle arc (tooth tip)
 * 4. Left involute curve (coast side)
 *
 * @param geom Calculated gear geometry
 * @param params Original parameters for fine control
 * @return Closed wire representing one tooth profile
 */
TopoDS_Wire createToothProfile(const GearGeometry& geom, const GearParams& params) {
    BRepBuilderAPI_MakeWire wireBuilder;

    // Calculate half tooth thickness angle at pitch circle (with backlash)
    double half_tooth_angle = (M_PI / geom.teeth) - 0.5 * geom.backlash_angle;

    // Generate involute curves for both sides of the tooth
    auto rightInvolute = generateInvolutePoints(geom.base_radius, geom.base_radius, geom.addendum_radius,
                                                params.involute_samples, false);

    auto leftInvolute =
        generateInvolutePoints(geom.base_radius, geom.base_radius, geom.addendum_radius, params.involute_samples, true);

    // Apply rotation to position the involutes correctly
    auto rotatePoint = [](gp_Pnt& point, double angle) {
        double cos_a = std::cos(angle);
        double sin_a = std::sin(angle);
        double x = point.X();
        double y = point.Y();
        point.SetX(cos_a * x - sin_a * y);
        point.SetY(sin_a * x + cos_a * y);
    };

    // Rotate involutes to create proper tooth thickness
    for (auto& point : rightInvolute) {
        rotatePoint(point, +half_tooth_angle);
    }
    for (auto& point : leftInvolute) {
        rotatePoint(point, -half_tooth_angle);
    }

    // Get key points for connecting arcs
    gp_Pnt baseRight = rightInvolute.front();
    gp_Pnt tipRight = rightInvolute.back();
    gp_Pnt tipLeft = leftInvolute.back();
    gp_Pnt baseLeft = leftInvolute.front();

    // 1. Root arc (if there's space between base circle and dedendum)
    if (geom.dedendum_radius < geom.base_radius - 1e-6) {
        double angleLeft = std::atan2(baseLeft.Y(), baseLeft.X());
        double angleRight = std::atan2(baseRight.Y(), baseRight.X());

        // Ensure proper arc direction
        if (angleRight > angleLeft) angleRight -= 2.0 * M_PI;

        gp_Circ rootCircle(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), geom.dedendum_radius);
        Handle(Geom_Circle) rootGeom = new Geom_Circle(rootCircle);

        // Create arc from right to left (counter-clockwise)
        gp_Pnt rootStart(geom.dedendum_radius * std::cos(angleRight), geom.dedendum_radius * std::sin(angleRight), 0);
        gp_Pnt rootEnd(geom.dedendum_radius * std::cos(angleLeft), geom.dedendum_radius * std::sin(angleLeft), 0);

        GC_MakeArcOfCircle arcBuilder(rootCircle, rootStart, rootEnd, Standard_False);
        if (arcBuilder.IsDone()) {
            wireBuilder.Add(BRepBuilderAPI_MakeEdge(arcBuilder.Value()));
        }

        // Connect root to base circle
        wireBuilder.Add(BRepBuilderAPI_MakeEdge(rootEnd, baseLeft));
    }

    // 2. Left involute curve
    Handle(Geom_BSplineCurve) leftSpline = createBSplineFromPoints(leftInvolute);
    wireBuilder.Add(BRepBuilderAPI_MakeEdge(leftSpline));

    // 3. Tip arc
    double tipAngleLeft = std::atan2(tipLeft.Y(), tipLeft.X());
    double tipAngleRight = std::atan2(tipRight.Y(), tipRight.X());

    if (tipAngleRight < tipAngleLeft) tipAngleRight += 2.0 * M_PI;

    gp_Circ tipCircle(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), geom.addendum_radius);
    GC_MakeArcOfCircle tipArcBuilder(tipCircle, tipLeft, tipRight, Standard_False);
    if (tipArcBuilder.IsDone()) {
        wireBuilder.Add(BRepBuilderAPI_MakeEdge(tipArcBuilder.Value()));
    }

    // 4. Right involute curve (reversed)
    std::reverse(rightInvolute.begin(), rightInvolute.end());
    Handle(Geom_BSplineCurve) rightSpline = createBSplineFromPoints(rightInvolute);
    wireBuilder.Add(BRepBuilderAPI_MakeEdge(rightSpline));

    // 5. Connect back to start if needed
    if (geom.dedendum_radius < geom.base_radius - 1e-6) {
        gp_Pnt rootStart(geom.dedendum_radius * std::atan2(baseRight.Y(), baseRight.X()),
                         geom.dedendum_radius * std::sin(std::atan2(baseRight.Y(), baseRight.X())), 0);
        wireBuilder.Add(BRepBuilderAPI_MakeEdge(baseRight, rootStart));
    }

    wireBuilder.Build();
    if (!wireBuilder.IsDone()) {
        throw std::runtime_error("Failed to create tooth profile wire");
    }

    return wireBuilder.Wire();
}

}  // anonymous namespace

// Public API implementation

geometry::ShapePtr MakeInvoluteGear(const GearParams& params) {
    try {
        // Calculate all gear geometry
        GearGeometry geom = GearGeometry::calculate(params);

        // Create the tooth profile as a wire
        TopoDS_Wire toothWire = createToothProfile(geom, params);

        // Create face from wire
        BRepBuilderAPI_MakeFace faceBuilder(toothWire);
        if (!faceBuilder.IsDone()) {
            throw std::runtime_error("Failed to create tooth face");
        }
        TopoDS_Face toothFace = faceBuilder.Face();

        // Extrude to create 3D tooth
        gp_Vec extrudeVector(0, 0, geom.thickness);
        BRepPrimAPI_MakePrism prismBuilder(toothFace, extrudeVector);
        TopoDS_Shape toothSolid = prismBuilder.Shape();

        // Create gear by copying and rotating the tooth
        TopoDS_Shape gearSolid = toothSolid;

        for (int i = 1; i < geom.teeth; ++i) {
            gp_Trsf rotation;
            rotation.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), i * geom.tooth_angle);

            BRepBuilderAPI_Transform transformer(toothSolid, rotation, Standard_True);
            TopoDS_Shape rotatedTooth = transformer.Shape();

            BRepAlgoAPI_Fuse fuseOp(gearSolid, rotatedTooth);
            if (!fuseOp.IsDone()) {
                throw std::runtime_error("Failed to fuse gear teeth");
            }
            gearSolid = fuseOp.Shape();
        }

        // Add center hub if dedendum radius is significantly larger than needed
        double hubRadius = geom.dedendum_radius;
        if (hubRadius > geom.base_radius * 0.9) {
            BRepPrimAPI_MakeCylinder hubBuilder(hubRadius, geom.thickness);
            TopoDS_Shape hub = hubBuilder.Shape();

            BRepAlgoAPI_Fuse hubFuse(gearSolid, hub);
            if (hubFuse.IsDone()) {
                gearSolid = hubFuse.Shape();
            }
        }

        // Create center bore if specified
        if (params.bore_diameter > 0) {
            double boreRadius = params.bore_diameter * 0.5;

            // Make bore slightly longer to ensure clean cut
            BRepPrimAPI_MakeCylinder boreBuilder(boreRadius, geom.thickness + 0.2);
            TopoDS_Shape bore = boreBuilder.Shape();

            // Translate bore to center it properly
            gp_Trsf translation;
            translation.SetTranslation(gp_Vec(0, 0, -0.1));
            BRepBuilderAPI_Transform boreTransformer(bore, translation, Standard_True);
            bore = boreTransformer.Shape();

            // Cut the bore
            BRepAlgoAPI_Cut cutOp(gearSolid, bore);
            if (!cutOp.IsDone()) {
                throw std::runtime_error("Failed to create center bore");
            }
            gearSolid = cutOp.Shape();
        }

        return std::make_shared<geometry::Shape>(gearSolid);

    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Gear generation failed: ") + e.what());
    }
}

// Legacy interface implementation
geometry::ShapePtr MakeInvoluteGear(int z, double m, double th, double bore, double pressureDeg) {
    GearParams params;
    params.teeth = z;
    params.module = m;
    params.thickness = th;
    params.bore_diameter = bore;
    params.pressure_angle_deg = pressureDeg;

    return MakeInvoluteGear(params);
}

}  // namespace mech
