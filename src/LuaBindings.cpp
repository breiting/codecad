#include "runtime/LuaBindings.hpp"

#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <STEPControl_Writer.hxx>
#include <StlAPI_Writer.hxx>
#include <TopoDS_Shape.hxx>

#include "geo/Boolean.hpp"
#include "geo/Extrude.hpp"
#include "geo/Features.hpp"
#include "geo/Gear.hpp"
#include "geo/Primitives.hpp"
#include "geo/Sketch.hpp"
#include "geo/Transform.hpp"
#include "io/Export.hpp"
#include "io/Manifest.hpp"
#include "runtime/Assembly.hpp"

namespace runtime {

using geo::ShapePtr;
using runtime::Assembly;
using runtime::Part;

LuaBindings::LuaBindings() = default;

// Helper: parse table like { {x,y}, {x,y}, ... } into std::vector<std::pair<double,double>>
static std::vector<std::pair<double, double>> parse_point_table(sol::table t) {
    std::vector<std::pair<double, double>> out;
    out.reserve(t.size());
    for (std::size_t i = 1; i <= t.size(); ++i) {
        sol::object row = t[i];
        if (row.get_type() == sol::type::table) {
            sol::table pairtbl = row.as<sol::table>();
            // allow both {x, y} and {x = ..., y = ...}
            sol::object ox = pairtbl[1];
            sol::object oy = pairtbl[2];
            if (!ox.valid() || !oy.valid()) {
                ox = pairtbl["x"];
                oy = pairtbl["y"];
            }
            if (!ox.valid() || !oy.valid() || !ox.is<double>() || !oy.is<double>()) {
                throw std::runtime_error("poly: each point must be {x, y} numbers");
            }
            out.emplace_back(ox.as<double>(), oy.as<double>());
        } else {
            throw std::runtime_error("poly: points must be tables like {x, y}");
        }
    }
    return out;
}

void LuaBindings::Register(sol::state& lua) {
    // Type: ShapePtr as userdata
    lua.new_usertype<geo::Shape>("__shape_usertype__", sol::no_constructor);

    auto asm_t = lua.new_usertype<Assembly>("Assembly");
    asm_t.set_function("get_name", [](Assembly& a) { return a.name; });
    asm_t.set_function("set_name", [](Assembly& a, const std::string& n) { a.name = n; });
    asm_t.set_function("count", [](Assembly& a) { return (int)a.parts.size(); });
    asm_t.set_function("get_part", [](Assembly& a, int i) {
        if (i < 1 || (size_t)i > a.parts.size()) return Part{};
        return a.parts[(size_t)i - 1];
    });
    asm_t.set_function("clear", [](Assembly& a) { a.parts.clear(); });

    auto part_t = lua.new_usertype<Part>("Part");
    part_t.set_function("get_name", [](Part& p) { return p.name; });
    part_t.set_function("set_name", [](Part& p, const std::string& n) { p.name = n; });
    part_t.set_function("get_shape", [](Part& p) { return p.shape; });
    part_t.set_function("set_shape", [](Part& p, const geo::ShapePtr& s) { p.shape = s; });
    part_t.set_function("get_ex", [](Part& p) { return p.ex; });
    part_t.set_function("set_ex", [](Part& p, double v) { p.ex = v; });
    part_t.set_function("get_ey", [](Part& p) { return p.ey; });
    part_t.set_function("set_ey", [](Part& p, double v) { p.ey = v; });
    part_t.set_function("get_ez", [](Part& p) { return p.ez; });
    part_t.set_function("set_ez", [](Part& p, double v) { p.ez = v; });

    sol::table params = lua.create_named_table("PARAMS");

    lua.set_function("param", [&lua](const std::string& name, sol::object def) -> sol::object {
        sol::table p = lua["PARAMS"];
        sol::object v = p[name];
        if (v.valid() && v.get_type() != sol::type::nil && v.get_type() != sol::type::none) {
            // Wenn default Zahl ist und v ein String (z. B. von -D), versuch zu parsen
            if (def.is<double>() && v.is<std::string>()) {
                try {
                    double d = std::stod(v.as<std::string>());
                    return sol::make_object(lua, d);
                } catch (...) { /* fallback unten */
                }
            }
            if (def.is<bool>() && v.is<std::string>()) {
                std::string s = v.as<std::string>();
                for (auto& c : s) c = std::tolower(c);
                bool b = (s == "1" || s == "true" || s == "yes" || s == "on");
                return sol::make_object(lua, b);
            }
            return v;  // sonst direkt zurückgeben (string, number, bool, etc.)
        }
        return def;  // kein Override -> default
    });

    lua.set_function("mm", [](double v) { return v; });
    lua.set_function("deg", [](double v) { return v; });

    // Primitives
    lua.set_function("box", [](double x, double y, double z) -> ShapePtr { return geo::MakeBox(x, y, z); });
    lua.set_function("cylinder", [](double d, double h) -> ShapePtr { return geo::MakeCylinder(d, h); });
    lua.set_function("hex_prism",
                     [](double across_flats, double h) -> ShapePtr { return geo::MakeHexPrism(across_flats, h); });
    // Boolean union
    lua.set_function("union", [](sol::variadic_args va) -> ShapePtr {
        std::vector<ShapePtr> shapes;
        shapes.reserve(va.size());
        for (auto v : va) {
            if (v.is<ShapePtr>()) {
                shapes.push_back(v.as<ShapePtr>());
            } else {
                throw std::runtime_error("union(...) expects shapes");
            }
        }
        if (shapes.size() < 2) {
            throw std::runtime_error("union(...) expects at least 2 shapes");
        }
        return geo::MakeUnion(shapes);
    });

    lua.set_function("difference",
                     [](const ShapePtr& a, const ShapePtr& b) -> ShapePtr { return geo::MakeDifference(a, b); });

    lua.set_function("assembly", [](const std::string& n) {
        Assembly a;
        a.name = n;
        return a;
    });
    lua.set_function(
        "part", [](const std::string& name, const geo::ShapePtr& s, sol::object ex, sol::object ey, sol::object ez) {
            Part p;
            p.name = name;
            p.shape = s;
            if (ex.valid()) p.ex = ex.as<double>();
            if (ey.valid()) p.ey = ey.as<double>();
            if (ez.valid()) p.ez = ez.as<double>();
            return p;
        });
    lua.set_function("add_part", [](Assembly& a, const Part& p) { a.parts.push_back(p); });
    lua.set_function("emit_assembly", [this](const Assembly& a) {
        // Speichere genauso wie 'emit' – optional
        this->m_Emitted = a.parts.empty() ? nullptr : a.parts.front().shape;
        // Schreibe Manifest + Files in __OUTDIR (Lua hat __OUTDIR gesetzt)
        // Hole __OUTDIR aus Lua-Globalen:
        // -> Wir hängen die Funktion so ein, dass sie 'package' kennt:
        // In sol2: Zugriff auf Globals über sol::state_view im Register nicht trivial.
        // Simpel: Wir lassen 'emit_assembly(a, outdir)' den Pfad bekommen:
    });

    lua.set_function("emit_assembly",
                     [](const Assembly& a, const std::string& outdir) { io::WriteAssemblyManifest(a, outdir, true); });

    // --- Sketch / Profiles ---
    lua.set_function("poly_xy", [](sol::table pts_tbl, sol::optional<bool> closed_opt) -> ShapePtr {
        auto pts = parse_point_table(pts_tbl);
        bool closed = closed_opt.value_or(true);
        return geo::PolylineXY_Face(pts, closed);
    });

    lua.set_function(
        "poly_xz",
        [](sol::table rz_tbl, sol::optional<bool> closed_opt, sol::optional<bool> close_to_axis_opt) -> ShapePtr {
            auto rz = parse_point_table(rz_tbl);
            bool closed = closed_opt.value_or(false);
            bool close_to_axis = close_to_axis_opt.value_or(false);
            return geo::PolylineXZ_Face(rz, closed, close_to_axis);
        });

    // --- Extrude / Revolve ---
    lua.set_function("extrude",
                     [](const ShapePtr& face, double height) -> ShapePtr { return geo::ExtrudeZ(face, height); });
    lua.set_function("revolve",
                     [](const ShapePtr& prof, double angle_deg) -> ShapePtr { return geo::RevolveZ(prof, angle_deg); });

    // --- Gear ---
    lua.set_function("gear_involute",
                     [](int z, double m, double th, double bore, sol::object pressure_deg) -> ShapePtr {
                         try {
                             double p = pressure_deg.valid() ? pressure_deg.as<double>() : 20.0;
                             return geo::MakeInvoluteGear(z, m, th, bore, p);
                         } catch (const Standard_Failure& e) {
                             throw std::runtime_error(std::string("gear_involute failed: ") + e.GetMessageString());
                         } catch (const std::exception& e) {
                             throw;
                         }
                     });

    // Transforms
    lua.set_function("translate", [](const ShapePtr& s, double dx, double dy, double dz) -> ShapePtr {
        return geo::Translate(s, dx, dy, dz);
    });
    lua.set_function("rot_x", [](const ShapePtr& s, double degs) -> ShapePtr { return geo::RotateX(s, degs); });
    lua.set_function("rot_y", [](const ShapePtr& s, double degs) -> ShapePtr { return geo::RotateY(s, degs); });
    lua.set_function("rot_z", [](const ShapePtr& s, double degs) -> ShapePtr { return geo::RotateZ(s, degs); });
    lua.set_function("scale", [](const ShapePtr& s, double factor) -> ShapePtr { return geo::Scale(s, factor); });

    // emit
    lua.set_function("emit", [this](const ShapePtr& s) { this->m_Emitted = s; });

    // fillet/chamfer
    lua.set_function("fillet", [](const ShapePtr& s, double radius_mm) -> ShapePtr {
        try {
            return geo::FilletAll(s, radius_mm);
        } catch (const Standard_Failure& e) {
            throw std::runtime_error(std::string("fillet failed: ") + e.GetMessageString());
        } catch (const std::exception& e) {
            throw;  // sol2 zeigt die std::exception message
        }
    });

    lua.set_function("chamfer", [](const ShapePtr& s, double distance_mm) -> ShapePtr {
        try {
            return geo::ChamferAll(s, distance_mm);
        } catch (const Standard_Failure& e) {
            throw std::runtime_error(std::string("chamfer failed: ") + e.GetMessageString());
        } catch (const std::exception& e) {
            throw;
        }
    });

    // save_stl/save_step
    lua.set_function("save_stl", [](const ShapePtr& s, const std::string& relpath) {
        std::string path = relpath;
        // Allow relative paths: join with __OUTDIR from Lua if not absolute
        return io::SaveSTL(s, path, 0.1);
    });

    lua.set_function("save_step", [](const ShapePtr& s, const std::string& relpath) {
        std::string path = relpath;
        return io::SaveSTEP(s, path);
    });
}

}  // namespace runtime
