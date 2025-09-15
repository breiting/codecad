#include "ccad/mech/Rod.hpp"

#include "ccad/feature/Chamfer.hpp"
#include "ccad/feature/Fillet.hpp"
#include "ccad/geom/Cylinder.hpp"
#include "ccad/mech/Threads.hpp"
#include "ccad/ops/Boolean.hpp"
#include "ccad/ops/Transform.hpp"

using namespace ccad::feature;
using namespace ccad::ops;

namespace ccad {
namespace mech {
Shape Rod(double diameter, double length, const RodSpec& spec) {
    auto rod = geom::Cylinder(diameter, length);
    if (spec.chamferBottom) {
        auto edges = select::EdgeSelector::FromShape(rod)   //
                         .onBoxSide(select::BoxSide::ZMin)  //
                         .collect();

        rod = Fillet(rod, edges, 1);
    }
    if (spec.chamferTop) {
        auto edges = select::EdgeSelector::FromShape(rod)   //
                         .onBoxSide(select::BoxSide::ZMax)  //
                         .collect();

        rod = Fillet(rod, edges, 1);
    }
    return rod;
}

Shape ThreadedRod(double totalLength, double threadLength, const RodSpec& rodSpec, const ThreadSpec& threadSpec) {
    double majorDiameter;
    auto rod = ThreadOps::ThreadExternalRod(threadSpec, totalLength, threadLength, majorDiameter);

    ChamferRadialSpec radSpec;
    radSpec.type = ChamferRadialType::External;

    if (rodSpec.chamferBottom || rodSpec.chamferTop) {
        auto cutter = ChamferCutterRadial(majorDiameter, radSpec);

        if (rodSpec.chamferBottom) {
            rod = Difference(rod, cutter);
        }
        if (rodSpec.chamferTop) {
            cutter = RotateX(cutter, 180.0);
            cutter = Translate(cutter, 0, 0, totalLength);
            rod = Difference(rod, cutter);
        }
    }
    return rod;
}
}  // namespace mech
}  // namespace ccad
