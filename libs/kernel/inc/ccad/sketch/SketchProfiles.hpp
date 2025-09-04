#pragma once
#include <ccad/base/Math.hpp>
#include <ccad/base/Shape.hpp>
#include <vector>

namespace ccad::sketch {

/**
 * @brief Build a 2D rectangular-ish freeform profile in the XY plane.
 *
 * Creates a closed Face from a polyline defined by points in XY (Z=0).
 * Duplicate consecutive points are ignored; the wire is closed automatically.
 *
 * @param pts  Polyline points in XY (must have size >= 3 after de-duplication).
 * @return Shape  Planar face in XY (Z=0).
 * @throws Exception on invalid/degenerate input.
 */
Shape PolyXY(const std::vector<Vec2>& pts);

/**
 * @brief Build a profile wire in the XZ plane, typically used for RevolveZ.
 *
 * Points lie in XZ (Y=0). If @p closed is true, the polyline is closed
 * explicitly (last → first). If @p closeToAxis is true, the function
 * appends segments that run to/from the Z-axis (x=0) to form a closed
 * “cup” shape suited for revolve (common CAD workflow).
 *
 * Notes:
 * - If both @p closed and @p closeToAxis are false, you get an open wire.
 * - If @p closeToAxis is true, closure is constructed as:
 *   (lastX,lastZ) → (0,lastZ) → (0,firstZ) → (firstX,firstZ).
 *
 * @param pts          Polyline points in XZ (must have size >= 2 after de-dup).
 * @param closed       Close wire back to first point.
 * @param closeToAxis  Close wire to the Z-axis (overrides/augments closure).
 * @return Shape  Wire in the XZ plane (Y=0).
 * @throws Exception on invalid/degenerate input.
 */
Shape ProfileXZ(const std::vector<Vec2>& pts, bool closed = false, bool closeToAxis = false);

}  // namespace ccad::sketch
