#include "ccad/mech/Rod.hpp"

#include "ccad/geom/Cylinder.hpp"
#include "ccad/mech/Threads.hpp"

namespace ccad {
namespace mech {
Shape Rod(const RodSpec& spec) {
    return geom::Cylinder(spec.diameter, spec.length);
}

Shape ThreadedRod(const RodSpec& rodSpec, const ThreadSpec& threadSpec) {
    return ThreadOps::ThreadExternalRod(threadSpec, rodSpec.length, rodSpec.length);
}
}  // namespace mech
}  // namespace ccad
