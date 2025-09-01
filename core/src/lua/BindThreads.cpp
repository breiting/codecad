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

    lua.set_function("bolt_M", [](double diameter, double pitch, double length, double head_height, double height_width,
                                  sol::optional<mech::ThreadOptions> opts) {
        mech::ThreadOptions o = opts.value_or(mech::ThreadOptions{});

        // Externe Schraube (M12 x 1.5)
        mech::CoarseThreadParams params;
        params.length = 20.0;  // 20mm lang
        params.depth = 1.0;    // 1mm Gewindehöhe
        params.turns = 10;     // 10 Umdrehungen
        params.flankAngleDeg = 60.0;

        return mech::CoarseThread::CreateExternalThread(15.0, params);
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
