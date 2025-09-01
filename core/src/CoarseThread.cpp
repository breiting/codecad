#include "mech/CoarseThread.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

// OCCT
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <Geom_BSplineCurve.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TopoDS.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>

#include "geometry/Shape.hpp"

namespace mech {

// =============================================================================
// PRIVATE HELPER FUNCTIONS
// =============================================================================

double CoarseThread::ClampToPositive(double value, double minValue) {
    return std::max(value, minValue);
}

Handle(Geom_Curve) CoarseThread::CreateHelixCurve(double pitchRadius, double pitch, double length, bool leftHand,
                                                  int segments) {
    if (pitchRadius <= 0.0 || pitch <= 0.0 || length <= 0.0) {
        throw std::invalid_argument("CreateHelixCurve: pitchRadius, pitch, and length must be positive");
    }

    // Calculate total angular span and number of points
    const double totalTurns = length / pitch;
    const double totalAngle = 2.0 * M_PI * totalTurns;
    const int totalPoints = std::max(16, static_cast<int>(segments * totalTurns));

    std::cout << "Sampled points: " << totalPoints << std::endl;

    // Angular step and Z advancement per unit angle
    const double angleStep = totalAngle / (totalPoints - 1);
    const double zAdvancePerRadian = (leftHand ? -1.0 : 1.0) * pitch / (2.0 * M_PI);

    // Generate helix points
    TColgp_Array1OfPnt helixPoints(1, totalPoints);
    for (int i = 0; i < totalPoints; ++i) {
        const double angle = i * angleStep;
        const double x = pitchRadius * std::cos(angle);
        const double y = pitchRadius * std::sin(angle);
        const double z = zAdvancePerRadian * angle;

        helixPoints.SetValue(i + 1, gp_Pnt(x, y, z));
    }

    // Create smooth B-spline curve through the points
    try {
        GeomAPI_PointsToBSpline splineBuilder(helixPoints,
                                              3,           // degree
                                              8,           // max degree
                                              GeomAbs_C2,  // C2 continuity
                                              1.0e-6);     // tolerance

        if (!splineBuilder.IsDone()) {
            throw std::runtime_error("Failed to create helix B-spline curve");
        }

        return splineBuilder.Curve();
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Error creating helix curve: " << e.what();
        throw std::runtime_error(ss.str());
    }
}

TopoDS_Wire CoarseThread::CreateTriangularProfile(double depth, double flankAngleDeg) {
    // Clamp flank angle to reasonable range for 3D printing
    const double clampedAngle = std::clamp(flankAngleDeg, 10.0, 120.0);
    const double halfAngleRad = 0.5 * (clampedAngle * M_PI / 180.0);

    // Calculate base width from triangle geometry
    const double halfBaseWidth = depth * std::tan(halfAngleRad);

    // Create isosceles triangle profile in YZ plane
    // - Triangle tip at (0, depth, 0) pointing radially outward (+Y)
    // - Base centered on Z-axis
    const gp_Pnt triangleTip(0.0, depth, 0.0);
    const gp_Pnt leftBase(0.0, 0.0, -halfBaseWidth);
    const gp_Pnt rightBase(0.0, 0.0, halfBaseWidth);

    // Create triangle edges
    const TopoDS_Edge leftFlank = BRepBuilderAPI_MakeEdge(triangleTip, leftBase);
    const TopoDS_Edge base = BRepBuilderAPI_MakeEdge(leftBase, rightBase);
    const TopoDS_Edge rightFlank = BRepBuilderAPI_MakeEdge(rightBase, triangleTip);

    // Assemble into closed wire
    try {
        BRepBuilderAPI_MakeWire wireBuilder;
        wireBuilder.Add(leftFlank);
        wireBuilder.Add(base);
        wireBuilder.Add(rightFlank);

        if (!wireBuilder.IsDone()) {
            throw std::runtime_error("Failed to create triangular profile wire");
        }

        return wireBuilder.Wire();
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Error creating triangular profile: " << e.what();
        throw std::runtime_error(ss.str());
    }
}

TopoDS_Wire CoarseThread::TransformProfileToPosition(const TopoDS_Wire& profile, double pitchRadius,
                                                     bool pointingInward) {
    try {
        // Transformation sequence:
        // 1. Rotate profile from YZ plane to position around cylinder
        // 2. Translate to correct radius position

        gp_Trsf transformation;

        if (pointingInward) {
            // For internal threads: rotate +90° so +Y becomes -X (pointing inward)
            transformation.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), M_PI / 2.0);

            // Translate outward so the profile base is positioned correctly
            // The profile tip should point inward from pitchRadius
            gp_Trsf translation;
            translation.SetTranslation(gp_Vec(-pitchRadius, 0.0, 0.0));
            transformation = translation * transformation;
        } else {
            // For external threads: rotate -90° so +Y becomes +X (pointing outward)
            transformation.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), -M_PI / 2.0);

            // Translate so profile base is at correct radial position
            // Profile tip points outward from pitchRadius
            gp_Trsf translation;
            translation.SetTranslation(gp_Vec(pitchRadius, 0.0, 0.0));
            transformation = translation * transformation;
        }

        BRepBuilderAPI_Transform transformer(profile, transformation, true);
        transformer.Build();

        if (!transformer.IsDone()) {
            throw std::runtime_error("Failed to transform profile to position");
        }

        return TopoDS::Wire(transformer.Shape());
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Error transforming profile: " << e.what();
        throw std::runtime_error(ss.str());
    }
}

TopoDS_Shape CoarseThread::CreateHelicalSweep(const TopoDS_Wire& helixPath, const TopoDS_Wire& profile) {
    try {
        // Create pipe shell along helix with Frenet frame
        // Frenet frame ensures the profile maintains proper orientation along the curved path
        BRepOffsetAPI_MakePipeShell pipeBuilder(helixPath);
        pipeBuilder.SetMode(true);  // Use Frenet frame
        pipeBuilder.Add(profile);
        pipeBuilder.Build();

        if (!pipeBuilder.IsDone()) {
            throw std::runtime_error("Failed to create helical sweep - pipe build failed");
        }

        // Convert to solid if possible
        pipeBuilder.MakeSolid();

        return pipeBuilder.Shape();
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Error creating helical sweep: " << e.what();
        throw std::runtime_error(ss.str());
    }
}

// =============================================================================
// PUBLIC INTERFACE IMPLEMENTATION
// =============================================================================

ThreadValidation CoarseThread::ValidateParameters(double outerOrBoreDiameter, const CoarseThreadParams& threadParams,
                                                  bool isExternal) {
    ThreadValidation result;
    std::stringstream errors;

    // Check basic parameter ranges
    if (outerOrBoreDiameter <= 0.0) {
        errors << "Diameter must be positive. ";
        result.isValid = false;
    }

    if (threadParams.length <= 0.0) {
        errors << "Length must be positive. ";
        result.isValid = false;
    }

    if (threadParams.depth <= 0.0) {
        errors << "Thread depth must be positive. ";
        result.isValid = false;
    }

    if (threadParams.turns < 1) {
        errors << "Number of turns must be at least 1. ";
        result.isValid = false;
    }

    if (threadParams.flankAngleDeg < 10.0 || threadParams.flankAngleDeg > 120.0) {
        errors << "Flank angle should be between 10° and 120° for practical threads. ";
        result.isValid = false;
    }

    if (threadParams.clearance < 0.0) {
        errors << "Clearance cannot be negative. ";
        result.isValid = false;
    }

    // Check geometric compatibility
    const double effectiveDiameter = isExternal ? (outerOrBoreDiameter - 2.0 * threadParams.clearance)
                                                : (outerOrBoreDiameter + 2.0 * threadParams.clearance);

    const double effectiveRadius = 0.5 * effectiveDiameter;

    if (isExternal) {
        // For external threads, core radius must be positive
        const double coreRadius = effectiveRadius - threadParams.depth;
        if (coreRadius <= 0.0) {
            errors << "Thread depth too large for given diameter - would create negative core radius. ";
            result.isValid = false;
        }
    } else {
        // For internal threads, check that we have reasonable geometry
        if (threadParams.depth >= effectiveRadius) {
            errors << "Thread depth too large for given bore diameter. ";
            result.isValid = false;
        }
    }

    // Check pitch calculation
    const double pitch = threadParams.length / threadParams.turns;
    if (pitch <= 0.0) {
        errors << "Calculated pitch is not positive (length/turns). ";
        result.isValid = false;
    }

    result.errorMessage = errors.str();
    return result;
}

double CoarseThread::CalculatePitch(const CoarseThreadParams& threadParams) {
    if (threadParams.turns <= 0) {
        throw std::invalid_argument("Cannot calculate pitch: turns must be positive");
    }
    return threadParams.length / static_cast<double>(threadParams.turns);
}

geometry::ShapePtr CoarseThread::CreateExternalThread(double outerDiameter, const CoarseThreadParams& threadParams) {
    // Validate parameters first
    auto validation = ValidateParameters(outerDiameter, threadParams, true);
    if (!validation) {
        throw std::invalid_argument("Invalid external thread parameters: " + validation.errorMessage);
    }

    try {
        // Calculate key dimensions
        const double pitch = CalculatePitch(threadParams);
        const double effectiveOuterDiameter = outerDiameter - 2.0 * std::max(0.0, threadParams.clearance);
        const double outerRadius = 0.5 * effectiveOuterDiameter;
        const double coreRadius = ClampToPositive(outerRadius - threadParams.depth);
        const double pitchRadius = outerRadius - 0.5 * threadParams.depth;

        // Create helix curve at pitch radius
        Handle(Geom_Curve) helixCurve = CreateHelixCurve(pitchRadius, pitch, threadParams.length, threadParams.leftHand,
                                                         threadParams.helixSegments);

        TopoDS_Wire helixWire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(helixCurve));

        // Create triangular thread profile
        TopoDS_Wire triangleProfile = CreateTriangularProfile(threadParams.depth, threadParams.flankAngleDeg);

        // Position profile at pitch radius, pointing outward
        TopoDS_Wire positionedProfile =
            TransformProfileToPosition(triangleProfile, pitchRadius - 0.5 * threadParams.depth, false);

        // Create thread ridges by sweeping profile along helix
        TopoDS_Shape threadRidges = CreateHelicalSweep(helixWire, positionedProfile);

        // Create core cylinder
        TopoDS_Shape coreCylinder = BRepPrimAPI_MakeCylinder(coreRadius, threadParams.length).Shape();

        // Fuse core and ridges to create complete external thread
        BRepAlgoAPI_Fuse fuseOperation(coreCylinder, threadRidges);
        fuseOperation.Build();

        if (!fuseOperation.IsDone()) {
            throw std::runtime_error("Failed to fuse core cylinder with thread ridges");
        }

        return std::make_shared<geometry::Shape>(fuseOperation.Shape());
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Failed to create external thread: " << e.what();
        throw std::runtime_error(ss.str());
    }
}

geometry::ShapePtr CoarseThread::CreateInternalThread(double boreDiameter, const CoarseThreadParams& threadParams) {
    // Validate parameters first
    auto validation = ValidateParameters(boreDiameter, threadParams, false);
    if (!validation) {
        throw std::invalid_argument("Invalid internal thread parameters: " + validation.errorMessage);
    }

    try {
        // Calculate key dimensions for internal thread cutter
        const double pitch = CalculatePitch(threadParams);
        const double effectiveBoreDiameter = boreDiameter + 2.0 * std::max(0.0, threadParams.clearance);
        const double boreRadius = 0.5 * effectiveBoreDiameter;

        // For internal threads, pitch radius is outside the bore
        const double pitchRadius = boreRadius + 0.5 * threadParams.depth;

        // Create helix curve at pitch radius
        Handle(Geom_Curve) helixCurve = CreateHelixCurve(pitchRadius, pitch, threadParams.length, threadParams.leftHand,
                                                         threadParams.helixSegments);

        TopoDS_Wire helixWire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(helixCurve));

        // Create triangular cutter profile
        TopoDS_Wire triangleProfile = CreateTriangularProfile(threadParams.depth, threadParams.flankAngleDeg);

        // Position profile pointing inward (toward bore center)
        TopoDS_Wire positionedProfile =
            TransformProfileToPosition(triangleProfile, pitchRadius + 0.5 * threadParams.depth, true);

        // Create thread cutter by sweeping profile along helix
        TopoDS_Shape threadCutter = CreateHelicalSweep(helixWire, positionedProfile);

        // Create central cylinder to remove the bore completely
        // Make it slightly larger to ensure clean boolean operation
        const double cutterRadius = boreRadius + threadParams.depth + 0.1;  // Extra margin for clean cut
        TopoDS_Shape centralCylinder = BRepPrimAPI_MakeCylinder(cutterRadius, threadParams.length).Shape();

        // Combine the helical cutter with central cylinder
        BRepAlgoAPI_Fuse fuseOperation(threadCutter, centralCylinder);
        fuseOperation.Build();

        if (!fuseOperation.IsDone()) {
            throw std::runtime_error("Failed to combine thread cutter with central cylinder");
        }

        return std::make_shared<geometry::Shape>(fuseOperation.Shape());
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Failed to create internal thread cutter: " << e.what();
        throw std::runtime_error(ss.str());
    }
}

}  // namespace mech
