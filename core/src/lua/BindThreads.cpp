#include "runtime/BindThreads.hpp"

#include <gp_XY.hxx>

#include "mech/Thread.hpp"

namespace runtime {
void RegisterThreads(sol::state& lua) {
    using mech::ThreadOpts;

    lua.new_usertype<ThreadOpts>(
        "ThreadOpts", sol::constructors<ThreadOpts()>(),

        // left: bool
        "left",
        sol::property([](const ThreadOpts& o) { return o.leftHand; }, [](ThreadOpts& o, bool v) { o.leftHand = v; }),

        // lead_in: double
        "lead_in",
        sol::property([](const ThreadOpts& o) { return o.leadIn; }, [](ThreadOpts& o, double v) { o.leadIn = v; }),

        // segments: int
        "segments",
        sol::property([](const ThreadOpts& o) { return o.segments; }, [](ThreadOpts& o, int v) { o.segments = v; }),

        // xy: double
        "xy", sol::property([](const ThreadOpts& o) { return o.xy; }, [](ThreadOpts& o, double v) { o.xy = v; }));

    lua.set_function("iso_coarse_pitch", &mech::IsoCoarsePitch);

    lua.set_function("metric_thread_external",
                     [](double d_major, double pitch, double length, sol::optional<mech::ThreadOpts> opts) {
                         mech::ThreadOpts o = opts.value_or(mech::ThreadOpts{});
                         return mech::MakeExternalMetricThread(d_major, pitch, length, o);
                     });

    lua.set_function("metric_thread_internal",
                     [](double d_nominal, double pitch, double height, sol::optional<mech::ThreadOpts> opts) {
                         mech::ThreadOpts o = opts.value_or(mech::ThreadOpts{});
                         return mech::MakeInternalMetricThread(d_nominal, pitch, height, o);
                     });
}

}  // namespace runtime
