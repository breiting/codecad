#include "Runtime/LuaBindings.hpp"

#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <STEPControl_Writer.hxx>
#include <StlAPI_Writer.hxx>
#include <TopoDS_Shape.hxx>

#include "Geo/Boolean.hpp"
#include "Geo/Primitives.hpp"
#include "Geo/Transform.hpp"
#include "IO/Export.hpp"

namespace Runtime {

using Geo::ShapePtr;

LuaBindings::LuaBindings() = default;

void LuaBindings::Register(sol::state& lua) {
    // Type: ShapePtr as userdata
    lua.new_usertype<Geo::Shape>("__shape_usertype__", sol::no_constructor);

    // Units (simple for MVP)
    lua.set_function("mm", [](double v) { return v; });
    lua.set_function("deg", [](double v) { return v; });

    // Primitives
    lua.set_function("box", [](double x, double y, double z) -> ShapePtr { return Geo::MakeBox(x, y, z); });
    lua.set_function("cylinder", [](double r, double h) -> ShapePtr { return Geo::MakeCylinder(r, h); });

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
        return Geo::MakeUnion(shapes);
    });

    lua.set_function("difference",
                     [](const ShapePtr& a, const ShapePtr& b) -> ShapePtr { return Geo::MakeDifference(a, b); });

    // Transforms
    lua.set_function("translate", [](const ShapePtr& s, double dx, double dy, double dz) -> ShapePtr {
        return Geo::Translate(s, dx, dy, dz);
    });
    lua.set_function("rot_x", [](const ShapePtr& s, double degs) -> ShapePtr { return Geo::RotateX(s, degs); });
    lua.set_function("rot_y", [](const ShapePtr& s, double degs) -> ShapePtr { return Geo::RotateY(s, degs); });
    lua.set_function("rot_z", [](const ShapePtr& s, double degs) -> ShapePtr { return Geo::RotateZ(s, degs); });
    lua.set_function("scale", [](const ShapePtr& s, double factor) -> ShapePtr { return Geo::Scale(s, factor); });

    // emit
    lua.set_function("emit", [this](const ShapePtr& s) { this->m_Emitted = s; });

    // save_stl/save_step
    lua.set_function("save_stl", [](const ShapePtr& s, const std::string& relpath) {
        std::string path = relpath;
        // Allow relative paths: join with __OUTDIR from Lua if not absolute
        return IO::SaveSTL(s, path, 0.1);
    });

    lua.set_function("save_step", [](const ShapePtr& s, const std::string& relpath) {
        std::string path = relpath;
        return IO::SaveSTEP(s, path);
    });
}

}  // namespace Runtime
