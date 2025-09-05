#include "runtime/BindPipes.hpp"

#include "geometry/Shape.hpp"
#include "mech/PipeTransition.hpp"

namespace runtime {

void RegisterPipes(sol::state& lua) {
    lua.set_function("pipe_transition", [](double rIn0, double rOut0, double rIn1, double rOut1, double length,
                                           sol::optional<double> steepnessOpt) {
        mech::PipeEnds ends{rIn0, rOut0, rIn1, rOut1, length};

        mech::PipeBlendOpts opts;
        opts.steepness = steepnessOpt.value_or(6.0);
        opts.kind = mech::PipeBlendOpts::Kind::Tanh;

        TopoDS_Shape s = mech::PipeTransition::Build(ends, opts);
        return std::make_shared<geometry::Shape>(s);
    });
}

}  // namespace runtime
