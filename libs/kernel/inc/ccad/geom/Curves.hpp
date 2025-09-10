#pragma once
#include <ccad/base/Math.hpp>
#include <ccad/base/Shape.hpp>
#include <vector>

namespace ccad::geom {

/**
 * @brief Specifies a lathe operation with a Bezier curve.
 * The curve will travel through the first and the last point.
 */
struct LatheSpec {
    std::vector<Vec2> points;  // XZ: x=radius, z=height
    double angleDeg{360.0};
    double thickness{0.0};  // 0 => solid, otherwise a hollow shape with wall thickness
};

/// \brief Build a surface of revolution from a 2D curve profile.
/// Supports Bezier and B-spline curves in XY plane.
/// Optionally creates hollow solids if thickness > 0.
Shape Lathe(const LatheSpec& spec);

/**
 * @brief Interpolate a C2 B-spline THROUGH given 2D points in XZ plane.
 * @param pts  List of sample points (must be >= 2).
 * @param degree  Spline degree in [2..5] (default 3).
 * @param tol     Geometric tolerance for interpolation (default 1e-6).
 * @return Shape holding a single 3D edge that lies in Y=0 (XZ plane).
 */
Shape BSplineXZ_Interpolate(const std::vector<Vec2>& pts, int degree = 3, double tol = 1e-6);

/**
 * @brief Build a (possibly rational) B-spline FROM control points (NURBS).
 *        Uses uniform knot spacing; periodic if requested.
 * @param ctrl     Control points (>= degree+1).
 * @param degree   Degree (default 3).
 * @param periodic If true, creates a periodic curve (closed & C^(degree-1)).
 * @param weights  Optional weights; if provided size must equal ctrl.size().
 * @return Shape edge in XZ.
 */
Shape BSplineXZ_Control(const std::vector<Vec2>& ctrl, int degree = 3, bool periodic = false,
                        const std::vector<double>* weights = nullptr);

/**
 * @brief Create a (possibly rational) Bézier curve FROM control points.
 *        Degree = ctrl.size()-1.
 * @param ctrl    Control points (>= 2).
 * @param weights Optional weights (same size as ctrl) → rational Bézier.
 * @return Shape edge in XZ.
 */
Shape BezierXZ(const std::vector<Vec2>& ctrl, const std::vector<double>* weights = nullptr);

/**
 * @brief Close a planar edge in XZ with a straight segment (start→end). Only adds an edge, no vertices.
 * @throws on non-edge input.
 */
Shape CloseCurve(const Shape& edge);

/**
 * @brief Close a curve shape (wire/edge) to the Z-axis to form a closed profile.
 *
 * Given an open curve in the XY plane (typically from BezierXY or BSplineXY_*),
 * this function connects the first and last points of the curve down to the
 * Z-axis (x=0) and closes the profile. The result is a closed Face that can be
 * revolved around Z to produce a solid of revolution.
 *
 * @param edge   Input curve (Shape containing TopoDS_Edge or Wire in XY).
 * @return Closed Shape (TopoDS_Face).
 */
Shape CloseCurveZ(const Shape& edge);

}  // namespace ccad::geom
