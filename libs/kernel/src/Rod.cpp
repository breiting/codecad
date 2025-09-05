#include "ccad/mech/Rod.hpp"

#include "ccad/feature/Chamfer.hpp"
#include "ccad/geom/Cylinder.hpp"
#include "ccad/mech/Threads.hpp"
#include "ccad/ops/Boolean.hpp"
#include "ccad/ops/Transform.hpp"

using namespace ccad::feature;
using namespace ccad::ops;

namespace ccad {
namespace mech {
Shape Rod(double diameter, double length, const RodSpec& spec) {
    return geom::Cylinder(diameter, length);
    // TODO: apply chamfer
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
