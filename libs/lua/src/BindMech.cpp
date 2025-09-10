#include <ccad/base/Shape.hpp>
#include <ccad/mech/PipeAdapter.hpp>
#include <sol/sol.hpp>

#include "ccad/lua/Bindings.hpp"
#include "ccad/mech/Rod.hpp"
#include "ccad/mech/Threads.hpp"

using namespace ccad;
using namespace ccad::mech;

namespace ccad {
namespace lua {

void RegisterMech(sol::state& lua) {
    lua.set_function("pipe_adapter", [](double rIn0, double rOut0, double rIn1, double rOut1, double length,
                                        sol::optional<double> steepnessOpt) {
        mech::PipeEnds ends{rIn0, rOut0, rIn1, rOut1, length};

        mech::PipeBlendOpts opts;
        opts.steepness = steepnessOpt.value_or(6.0);
        opts.kind = mech::PipeBlendOpts::Kind::Tanh;

        return mech::PipeAdapter(ends, opts);
    });

    lua.set_function(
        "rod", [](double diameter, double length, sol::optional<bool> chamferBottom, sol::optional<bool> chamferTop) {
            bool cBottom = chamferBottom.value_or(false);
            bool cTop = chamferTop.value_or(false);

            mech::RodSpec spec{cBottom, cTop};
            return mech::Rod(diameter, length, spec);
        });

    lua.new_enum("Handedness", "Right", Handedness::Right, "Left", Handedness::Left);
    lua.new_enum("TipStyle", "Sharp", TipStyle::Sharp, "Cut", TipStyle::Cut);

    lua.new_usertype<ThreadSpec>(
        "ThreadSpec", sol::constructors<ThreadSpec()>(),

        "fitDiameter",
        sol::property([](ThreadSpec& s) { return s.fitDiameter; }, [](ThreadSpec& s, double v) { s.fitDiameter = v; }),
        "pitch", sol::property([](ThreadSpec& s) { return s.pitch; }, [](ThreadSpec& s, double v) { s.pitch = v; }),
        "depth", sol::property([](ThreadSpec& s) { return s.depth; }, [](ThreadSpec& s, double v) { s.depth = v; }),
        "clearance",
        sol::property([](ThreadSpec& s) { return s.clearance; }, [](ThreadSpec& s, double v) { s.clearance = v; }),
        "segmentsPerTurn",
        sol::property([](ThreadSpec& s) { return s.segmentsPerTurn; },
                      [](ThreadSpec& s, int v) { s.segmentsPerTurn = v; }),
        "handed",
        sol::property([](ThreadSpec& s) { return s.handedness == Handedness::Left ? "left" : "right"; },
                      [](ThreadSpec& s, std::string_view v) {
                          s.handedness = (v == "left") ? Handedness::Left : Handedness::Right;
                      }),
        "tip",
        sol::property(
            [](ThreadSpec& s) { return s.tip == TipStyle::Sharp ? "sharp" : "cut"; },
            [](ThreadSpec& s, std::string_view v) { s.tip = (v == "sharp") ? TipStyle::Sharp : TipStyle::Cut; }),
        "tipCutRatio",
        sol::property([](ThreadSpec& s) { return s.tipCutRatio; }, [](ThreadSpec& s, double v) { s.tipCutRatio = v; }),
        "normalize", &ThreadSpec::Normalize);

    lua["ThreadSpec"]["new"] = []() { return ThreadSpec{}; };
    lua["ThreadSpec"][sol::meta_function::call] = []() { return ThreadSpec{}; };

    lua.set_function("threaded_rod", [](double totalLength, double threadLength, ThreadSpec threadSpec) {
        double majorDiameter = 0.0;
        auto shape = mech::ThreadOps::ThreadExternalRod(threadSpec, totalLength, threadLength, majorDiameter);
        return std::make_tuple(shape, majorDiameter);
    });
}
}  // namespace lua
}  // namespace ccad
