#pragma once
#include <TopoDS_Shape.hxx>

namespace mech {

/**
 * @brief Apply conical end chamfers to a Z-aligned threaded solid.
 *
 * We subtract a revolved triangular "ring wedge" at z=0 (start) and/or z=L (end),
 * producing a straight chamfer with a given length and angle.
 *
 * @param threaded      The threaded solid (axis along +Z).
 * @param outerRadius   Outer radius (at crest) used to size the chamfer cutter.
 * @param lengthZ       Chamfer length along Z (mm).
 * @param angleDeg      Chamfer face angle (degrees) relative to Z (e.g. 45).
 * @param totalLengthZ  Total model length along Z (mm) to place the end cutter.
 * @param start         If true, chamfer at z ∈ [0, lengthZ].
 * @param end           If true, chamfer at z ∈ [totalLengthZ - lengthZ, totalLengthZ].
 * @return TopoDS_Shape The chamfered solid (same orientation as input).
 *
 * @note This performs boolean CUTs. Use modest deflection/angles when meshing afterwards.
 */
TopoDS_Shape ChamferThreadEndsExternal(const TopoDS_Shape& threaded, double outerRadius, double lengthZ,
                                       double angleDeg, double totalLengthZ, bool start, bool end);

/**
 * @brief Apply internal entry chamfers to a Z-aligned nut
 *
 * We subtract a revolved triangular "inner ring wedge" at z=0 (start) and/or z=L (end),
 * producing a straight chamfer that opens the bore slightly towards inside.
 *
 * @param nut           The solid (already containing internal thread), axis along +Z.
 * @param innerRadius   Bore radius at the entry (at z=0 / z=L) before chamfer (mm).
 * @param lengthZ       Chamfer length along Z (mm).
 * @param angleDeg      Chamfer face angle (degrees) relative to Z (e.g. 45).
 * @param totalLengthZ  Total model length along Z (mm) to place the end cutter.
 * @param start         If true, chamfer at z ∈ [0, lengthZ].
 * @param end           If true, chamfer at z ∈ [totalLengthZ - lengthZ, totalLengthZ].
 * @return TopoDS_Shape Chamfered nut solid.
 */
TopoDS_Shape ChamferThreadEndsInternal(const TopoDS_Shape& nut, double innerRadius, double lengthZ, double angleDeg,
                                       double totalLengthZ, bool start, bool end);
}  // namespace mech
