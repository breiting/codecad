---@meta
---@module "ccad.core"
--[[
ccad.core – Low-level CodeCAD API exposed from C++ (sol2)

Conventions
- Units are millimeters [mm] unless stated otherwise.
- Angles are degrees unless stated otherwise.
- All modeling functions are pure (return new Shapes; do not mutate inputs).
- Shapes live in a right-handed coordinate system (Z up).

This file provides *stubs* for the Lua Language Server (completion & types).
It is not executed at runtime.
]]

--==============================================================
-- Fundamental types
--==============================================================

---@class Shape
-- Abstract handle to a solid/shell/face produced by the engine.

---@class Vec3
---@field x number
---@field y number
---@field z number

---@class BBox
---@field valid  boolean                 # True if the box is valid (non-empty)
---@field min    Vec3                    # Minimum corner (x_min, y_min, z_min)
---@field max    Vec3                    # Maximum corner (x_max, y_max, z_max)
---@field size   Vec3                    # Size along axes (width, depth, height)
---@field center Vec3                    # Box center in world coordinates

--==============================================================
-- PRIMITIVES
--==============================================================

--- Create an axis-aligned rectangular box with its lower corner at the origin.
--- Use `center_*` helpers to center afterwards if needed.
---@param x number Width along X [mm]
---@param y number Depth along Y [mm]
---@param z number Height along Z [mm]
---@return Shape
function box(x, y, z) end

--- Create a right circular cylinder whose axis is +Z, base at Z=0.
---@param d number Outer diameter [mm]
---@param h number Height along Z [mm]
---@return Shape
function cylinder(d, h) end

--- Create a cone (or truncated cone/frustum) whose axis is +Z, base at Z=0.
---@param d1 number Base diameter at Z=0 [mm]
---@param d2 number Top diameter at Z=h [mm]
---@param h  number Height along Z [mm]
---@return Shape
function cone(d1, d2, h) end

--- Create a wedge (a box with one X-edge sloped), axis-aligned at Z=0.
--- Useful for slope/ramps. `ltx` shifts the top-front edge along X.
---@param dx  number Size along X [mm]
---@param dy  number Size along Y [mm]
---@param dz  number Size along Z [mm]
---@param ltx number X-offset of the top edge (positive → taper towards -X) [mm]
---@return Shape
function wedge(dx, dy, dz, ltx) end

--- Create a solid sphere centered at the origin.
---@param d number Diameter [mm]
---@return Shape
function sphere(d) end

--- Create a hexagonal prism (flat-to-flat across X/Y), axis +Z, base at Z=0.
---@param across_flats number Distance across flats [mm]
---@param h            number Height along Z [mm]
---@return Shape
function hex_prism(across_flats, h) end

--==============================================================
-- TRANSFORMS
--==============================================================

--- Translate a shape by the given offsets in world coordinates.
---@param s  Shape
---@param dx number X offset [mm]
---@param dy number Y offset [mm]
---@param dz number Z offset [mm]
---@return Shape
function translate(s, dx, dy, dz) end

--- Rotate a shape around the world X-axis (right-hand rule).
---@param s   Shape
---@param deg number Angle in degrees
---@return Shape
function rotate_x(s, deg) end

--- Rotate a shape around the world Y-axis (right-hand rule).
---@param s   Shape
---@param deg number Angle in degrees
---@return Shape
function rotate_y(s, deg) end

--- Rotate a shape around the world Z-axis (right-hand rule).
---@param s   Shape
---@param deg number Angle in degrees
---@return Shape
function rotate_z(s, deg) end

--- Uniformly scale a shape around the world origin.
--- (For centered scaling use `center_*` first, then translate back.)
---@param s      Shape
---@param factor number Scale factor (1.0 = no change)
---@return Shape
function scale(s, factor) end

--==============================================================
-- BOOLEANS
--==============================================================

--- Union (OR) of two or more shapes.
---@param a Shape
---@param b Shape
---@param ... Shape  @optional additional shapes
---@return Shape
function union(a, b, ...) end

--- Subtract `b` from `a` (A \ B).
---@param a Shape
---@param b Shape
---@return Shape
function difference(a, b) end

--- Intersection (AND) of two shapes.
---@param a Shape
---@param b Shape
---@return Shape
function intersection(a, b) end

--==============================================================
-- FEATURES
--==============================================================

--- Apply a constant-radius edge fillet to all eligible sharp edges of `s`.
--- For selective control, consider pre-splitting geometry or using filters (future).
---@param s Shape
---@param r number Radius [mm]
---@return Shape
function fillet(s, r) end

--- Apply a constant-distance chamfer to all eligible sharp edges of `s`.
---@param s Shape
---@param d number Distance [mm]
---@return Shape
function chamfer(s, d) end

--==============================================================
-- DRAFT (2D outline extraction)
--==============================================================

--- Compute a planar section outline of a solid for drawing/CAM.
--- Cuts the shape with a plane parallel to the given axis at `value`
--- (e.g., axis="z", value=10 cuts at Z=10). Curves are approximated.
---@param s     Shape
---@param axis  '"x"'|'"y"'|'"z"' Which axis to keep (plane normal is that axis)
---@param value number Plane offset along that axis [mm]
---@param defl  number Curve deflection tolerance (smaller → finer) [mm]
---@return table points  # Array of polylines (each: array of {x=,y=} or {x=,y=,z=})
function section_outline(s, axis, value, defl) end

--==============================================================
-- SKETCH (2D profiles)
--==============================================================

--- Build a planar polygonal face in the XY-plane from points.
--- The polygon is closed by default.
---@param pts    ({[1]:number,[2]:number}|{x:number,y:number})[]  Array of 2D points
---@param closed? boolean                                         Defaults to true
---@return Shape                                                  -- planar face
function poly_xy(pts, closed) end

--- Build a planar polyline in the XZ-plane (useful for revolved profiles).
--- By default it is *open*. With `close_to_axis=true`, an open profile
--- is closed by connecting its ends vertically to the Z-axis (for revolve).
---@param pts            ({[1]:number,[2]:number}|{x:number,z:number})[]  Array of XZ points
---@param closed?        boolean                                         Defaults to false
---@param close_to_axis? boolean                                         Close to Z-axis if true
---@return Shape                                                         -- planar face/edge profile
function poly_xz(pts, closed, close_to_axis) end

--==============================================================
-- CONSTRUCTION (solid from sketch)
--==============================================================

--- Linear extrusion of a planar face/profile along +Z.
---@param face   Shape   Planar face (from poly_*)
---@param height number  Extrusion height [mm]
---@return Shape
function extrude(face, height) end

--- Revolve a profile around the Z-axis through the origin.
--- Use `poly_xz` to define the profile in the XZ-plane.
---@param profile   Shape   Profile in XZ-plane
---@param angle_deg number  Sweep angle in degrees (0–360)
---@return Shape
function revolve(profile, angle_deg) end

--==============================================================
-- IO
--==============================================================

--- Mark a shape as the model result for the current run (for live/CLI).
---@param s Shape
function emit(s) end

--- Save a shape as STL (triangulated).
---@param s    Shape
---@param path string Absolute or relative filesystem path
function save_stl(s, path) end

--- Save a shape as STEP (B-Rep).
---@param s    Shape
---@param path string Absolute or relative filesystem path
function save_step(s, path) end

--==============================================================
-- HELPERS
--==============================================================

--- Declare or read a parameter for parametric designs.
--- If `PARAMS[name]` is set (by project/CLI), it is coerced when possible;
--- otherwise `default` is returned.
---@param name    string
---@param default any
---@return any
function param(name, default) end

--- Identity helper for millimeters (semantic sugar).
---@param v number
---@return number
function mm(v) end

--- Identity helper for degrees (semantic sugar).
---@param v number
---@return number
function deg(v) end

--==============================================================
-- MEASURE / CENTERING
--==============================================================

--- Compute an axis-aligned bounding box of a shape.
---@param s                Shape
---@param useTriangulation? boolean  If true, box is computed from mesh (slower/finer)
---@return BBox
function bbox(s, useTriangulation) end

--- Center a shape along X around 0 (translates by half width).
---@param s Shape
---@return Shape
function center_x(s) end

--- Center a shape along Y around 0 (translates by half depth).
---@param s Shape
---@return Shape
function center_y(s) end

--- Center a shape along Z around 0 (translates by half height).
---@param s Shape
---@return Shape
function center_z(s) end

--- Center a shape in the XY-plane (X and Y only).
---@param s Shape
---@return Shape
function center_xy(s) end

--- Center a shape in all axes (X, Y, Z).
---@param s Shape
---@return Shape
function center_xyz(s) end

--- Translate a shape so that its center goes to the given coordinates.
---@param s  Shape
---@param cx number Target center X [mm]
---@param cy number Target center Y [mm]
---@param cz number Target center Z [mm]
---@return Shape
function center_to(s, cx, cy, cz) end

--==============================================================
-- GEARS (mechanical)
--==============================================================

--- Generate a standard involute spur gear (ISO-style).
--- Validates geometric constraints (e.g., module/teeth result in positive root).
---@param z            integer Number of teeth (z ≥ 6 typical)
---@param module       number  Module [mm] (pitch diameter / z)
---@param thickness    number  Gear thickness along Z [mm]
---@param bore         number  Center hole diameter [mm]
---@param pressure_deg? number  Pressure angle in degrees (default 20)
---@return Shape
function gear_involute(z, module, thickness, bore, pressure_deg) end

--- Print-friendly metric thread helpers (external/internal)
---@class ThreadOpts
---@field left boolean?   Left-hand thread (default false)
---@field xy number?      Radial XY compensation for printing in mm (default 0.15)
---@field lead_in number? Lead-in chamfer length in mm (default 1.5)
---@field segments integer? Helix tessellation density (default 96)

--- Lookup coarse ISO pitch for a nominal diameter (e.g. M6 -> 1.0).
---@param d_nominal number @Nominal diameter in mm (e.g. 6.0 for M6)
---@return number    @Pitch in mm
function iso_coarse_pitch(d_nominal) end

--- Build a print-friendly *external* metric thread (male).
--- The returned shape is a solid (core + thread fused).
---@param d_major number    @Major (outside) diameter in mm
---@param pitch number      @Pitch in mm
---@param length number     @Threaded length along Z in mm
---@param opts ThreadOpts?  @Optional print tuning (xy, lead_in, left, segments)
---@return Shape
function metric_thread_external(d_major, pitch, length, opts) end

--- Build a print-friendly *internal* metric thread (female).
--- The returned shape is a solid bore cut with thread volume.
---@param d_nominal number  @Nominal diameter (e.g. 6.0 for M6)
---@param pitch number      @Pitch in mm
---@param height number     @Thread height along Z in mm
---@param opts ThreadOpts?  @Optional print tuning (xy, lead_in, left, segments)
---@return Shape
function metric_thread_internal(d_nominal, pitch, height, opts) end

--==============================================================
-- PARAMETER TABLE (injected by engine)
--==============================================================

--- Global parameter table used by `param(name, default)`.
--- Values may be strings from CLI/GUI and are coerced by the engine when possible.
---@type table<string, any>
PARAMS = {}
