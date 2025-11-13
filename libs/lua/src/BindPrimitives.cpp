#include <ccad/base/PoissonDisk.hpp>
#include <ccad/base/Shape.hpp>
#include <ccad/geom/Box.hpp>
#include <ccad/geom/Cone.hpp>
#include <ccad/geom/Cylinder.hpp>
#include <ccad/geom/HexPrism.hpp>
#include <ccad/geom/Sphere.hpp>
#include <ccad/geom/Wedge.hpp>
#include <sol/sol.hpp>

#include "ccad/geom/Poisson.hpp"
#include "ccad/lua/Bindings.hpp"

using namespace ccad::geom;

namespace ccad {
namespace lua {

void RegisterPrimitives(sol::state& lua) {
    lua.set_function("box", [](double x, double y, double z) -> Shape { return Box(x, y, z); });

    lua.set_function("cylinder", [](double d, double h) -> Shape { return Cylinder(d, h); });
    lua.set_function("cone", [](double d1, double d2, double h) -> Shape { return Cone(d1, d2, h); });
    lua.set_function("wedge",
                     [](double dx, double dy, double dz, double ltx) -> Shape { return Wedge(dx, dy, dz, ltx); });
    lua.set_function("sphere", [](double d) -> Shape { return Sphere(d); });
    lua.set_function("hex_prism", [](double acrossFlats, double h) -> Shape { return HexPrism(acrossFlats, h); });

    // Poisson Plate
    lua.new_usertype<PoissonDiskSpec>(
        "PoissonDiskSpec", sol::constructors<PoissonDiskSpec()>(),

        "width",
        sol::property([](PoissonDiskSpec& s) { return s.width; }, [](PoissonDiskSpec& s, double v) { s.width = v; }),
        "height",
        sol::property([](PoissonDiskSpec& s) { return s.height; }, [](PoissonDiskSpec& s, double v) { s.height = v; }),
        "margin",
        sol::property([](PoissonDiskSpec& s) { return s.margin; }, [](PoissonDiskSpec& s, double v) { s.margin = v; }),
        "target_points",
        sol::property([](PoissonDiskSpec& s) { return s.targetPoints; },
                      [](PoissonDiskSpec& s, int v) { s.targetPoints = v; }),
        "r_min",
        sol::property([](PoissonDiskSpec& s) { return s.rMin; }, [](PoissonDiskSpec& s, double v) { s.rMin = v; }),
        "r_max",
        sol::property([](PoissonDiskSpec& s) { return s.rMax; }, [](PoissonDiskSpec& s, double v) { s.rMax = v; }),
        "min_gap",
        sol::property([](PoissonDiskSpec& s) { return s.minGap; }, [](PoissonDiskSpec& s, double v) { s.minGap = v; }),
        "seed",
        sol::property([](PoissonDiskSpec& s) { return s.seed; }, [](PoissonDiskSpec& s, uint64_t v) { s.seed = v; }),
        "radius_falloff",
        sol::property([](PoissonDiskSpec& s) { return s.radiusFalloff; },
                      [](PoissonDiskSpec& s, double v) { s.radiusFalloff = v; }),
        "density_falloff",
        sol::property([](PoissonDiskSpec& s) { return s.densityFalloff; },
                      [](PoissonDiskSpec& s, double v) { s.densityFalloff = v; }));

    lua["PoissonDiskSpec"]["new"] = []() { return PoissonDiskSpec{}; };
    lua["PoissonDiskSpec"][sol::meta_function::call] = []() { return PoissonDiskSpec{}; };

    lua.set_function("poisson_plate", [](PoissonDiskSpec spec, double thickness) {
        PoissonDiskGenerator gen(spec);
        PoissonResult res = gen.Run();
        return Poisson(spec, thickness, res);
    });
}
}  // namespace lua
}  // namespace ccad
