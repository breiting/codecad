#include "runtime/BindThreads.hpp"

#include <BRepPrimAPI_MakeCylinder.hxx>
#include <gp_XY.hxx>

#include "mech/CoarseThread.hpp"
#include "mech/Thread.hpp"

namespace runtime {
void RegisterThreads(sol::state& lua) {
    using mech::ThreadOptions;

    // TODO: expose new user type!!!
    // lua.new_usertype<ThreadOptions>(
    //     "ThreadOptions", sol::constructors<ThreadOptions()>(),
    //
    //     // left: bool
    //     "left",
    //     sol::property([](const ThreadOptions& o) { return o.leftHand; }, [](ThreadOptions& o, bool v) { o.leftHand =
    //     v; }),
    //
    //     // lead_in: double
    //     "lead_in",
    //     sol::property([](const ThreadOptions& o) { return o.leadIn; }, [](ThreadOptions& o, double v) { o.leadIn = v;
    //     }),
    //
    //     // segments: int
    //     "segments",
    //     sol::property([](const ThreadOptions& o) { return o.segments; }, [](ThreadOptions& o, int v) { o.segments =
    //     v; }),
    //
    //     // xy: double
    //     "xy", sol::property([](const ThreadOptions& o) { return o.xy; }, [](ThreadOptions& o, double v) { o.xy = v;
    //     }));
    //
    // lua.set_function("iso_coarse_pitch", &mech::IsoCoarsePitch);

    lua.set_function("bolt", [](double diameter, double length, sol::optional<mech::ThreadOptions> opts) {
        mech::ThreadOptions o = opts.value_or(mech::ThreadOptions{});

        mech::CoarseThreadParams p;
        p.length = 30.0;
        p.turns = 2;  // exakt ~1 Umdrehung
        p.depth = 3.5;
        p.clearance = 0.15;
        p.flankAngleDeg = 60.0;
        p.leftHand = false;
        p.tip = mech::ThreadTip::Cut;  // flache Spitze, robuster zu drucken
        p.tipFlatRatio = 0.5;

        return mech::CoarseThread::MakeBolt(24, p.length, 1.6 * 24, 0.7 * diameter, p);
    });

    lua.set_function("nut", [](double diameter, double length, sol::optional<mech::ThreadOptions> opts) {
        mech::ThreadOptions o = opts.value_or(mech::ThreadOptions{});

        mech::CoarseThreadParams p;
        p.length = length;
        p.turns = 10;
        p.depth = 0.6;
        p.clearance = 0.0;
        p.tip = mech::ThreadTip::Cut;
        p.tipFlatRatio = 0.5;

        return mech::CoarseThread::Test(p);
    });

    lua.set_function("metric_thread_external",
                     [](double d_major, double pitch, double length, sol::optional<mech::ThreadOptions> opts) {
                         mech::ThreadOptions o = opts.value_or(mech::ThreadOptions{});
                         return mech::MakeExternalMetricThread(d_major, pitch, length, o);
                     });

    lua.set_function("metric_thread_internal",
                     [](double d_nominal, double pitch, double height, sol::optional<mech::ThreadOptions> opts) {
                         mech::ThreadOptions o = opts.value_or(mech::ThreadOptions{});
                         return mech::MakeInternalMetricThread(d_nominal, pitch, height, 10.0, o);  // TODO: 10 as test
                     });
}

}  // namespace runtime
