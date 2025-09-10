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

---@class Vec2
---@field x number
---@field y number

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

---@class LatheOptions
---@field points ({[1]:number,[2]:number})[]  -- control points in XZ (x=radius, Z=height)
---@field angle? number                -- revolve angle in degrees (default 360)
---@field thickness? number            -- wall thickness (mm); if set → hollow body, else solid

--- Revolve an XY silhouette around Z to create a solid or hollow body.
--- If `thickness` is provided, an inner silhouette is auto-generated and subtracted.
---@param opts LatheOptions
---@return Shape
function lathe(opts) end

---Create a doubly-curved plate by fitting a BSpline surface to a height field z(x,y),
---offsetting by ±thickness/2, and closing the sides with ruled faces.
---Returns a watertight solid suitable for booleans and export.
---@param size_x number
---@param size_y number
---@param thickness number
---@param k_u number
---@param k_v number
---@return Shape
function curved_plate_xy(size_x, size_y, thickness, k_u, k_v) end

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
function fillet_all(s, r) end

--- Apply a constant-distance chamfer to all eligible sharp edges of `s`.
---@param s Shape
---@param d number Distance [mm]
---@return Shape
function chamfer_all(s, d) end

---@class EdgeSet
local EdgeSet = {}

---@class EdgeQuery
local EdgeQuery = {}

--- Start a new edge-query builder (optionally with a source shape).
---@overload fun(): EdgeQuery
---@param shape Shape
---@return EdgeQuery
function edges(shape) end

---@param shape Shape
---@return EdgeQuery
function EdgeQuery:from(shape) end

--- Filter edges lying on a specific AABB side of the shape's bounding box.
--- Valid sides: "xmin","xmax","ymin","ymax","zmin","zmax".
---@param side '"xmin"'|'"xmax"'|'"ymin"'|'"ymax"'|'"zmin"'|'"zmax"'
---@param tolerance_mm? number  -- reserved for future use
---@return EdgeQuery
function EdgeQuery:on_box_side(side, tolerance_mm) end

--- Filter by geometric type.
--- "line" selects straight edges, "circle" selects circular arcs.
---@param kind '"line"'|'"circle"'
---@return EdgeQuery
function EdgeQuery:geom(kind) end

--- Filter edges whose tangent direction is parallel to an axis within a tolerance.
---@param axis '"x"'|'"y"'|'"z"'
---@param tol_deg? number @angular tolerance in degrees (default 3°)
---@return EdgeQuery
function EdgeQuery:parallel(axis, tol_deg) end

--- Filter by dihedral angle between adjacent faces (in degrees).
---@param min_deg number
---@param max_deg number
---@return EdgeQuery
function EdgeQuery:dihedral_between(min_deg, max_deg) end

--- Filter by edge length interval (in mm).
---@param min_mm number
---@param max_mm number
---@return EdgeQuery
function EdgeQuery:length_between(min_mm, max_mm) end

--- Finalize and return the selected set of edges.
---@return EdgeSet
function EdgeQuery:collect() end

--- Apply a rolling fillet on the selected edges of `shape`.
---@param shape Shape
---@param edges EdgeSet
---@param radius_mm number
---@return Shape
function fillet(shape, edges, radius_mm) end

--- Apply a straight chamfer on the selected edges of `shape`.
---@param shape Shape
---@param edges EdgeSet
---@param distance_mm number
---@return Shape
function chamfer(shape, edges, distance_mm) end

--==============================================================
-- SKETCH (2D profiles)
--==============================================================

--- Build a planar polygonal face in the XY-plane from points.
--- The polygon is closed by default.
---@param pts    ({[1]:number,[2]:number}|{x:number,y:number})[]  Array of 2D points
---@return Shape                                                  -- planar face
function poly_xy(pts) end

--- Build a planar polyline in the XZ-plane (useful for revolved profiles).
--- By default it is *open*. With `close_to_axis=true`, an open profile
--- is closed by connecting its ends vertically to the Z-axis (for revolve).
---@param pts            ({[1]:number,[2]:number}|{x:number,z:number})[]  Array of XZ points
---@param closed?        boolean                                         Defaults to false
---@return Shape                                                         -- planar face/edge profile
function profile_xz(pts, closed) end

--- Draw a simple rectangle on XY plane.
---@param w  number  Width
---@param h  number Height
---@return Shape
function rect(w, h) end

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
---@return BBox
function bbox(s) end

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
-- Mechanical
--==============================================================

--- Build a smooth revolved transition between two pipes.
--- Radii are in millimeters; z-axis is the pipe axis; result is a Solid.
---@param rIn0 number       -- inner radius at z=0
---@param rOut0 number      -- outer radius at z=0
---@param rIn1 number       -- inner radius at z=length
---@param rOut1 number      -- outer radius at z=length
---@param length number     -- axial distance (mm)
---@param steepness? number -- smoothing factor between [4..12]
---@return Shape
function pipe_adapter(rIn0, rOut0, rIn1, rOut1, length, steepness) end

--==========================================================================
-- Enums
--==========================================================================

---@enum Handedness
-- Thread winding direction.
-- - "Right": standard right-hand thread (turn clockwise to tighten).
-- - "Left":  left-hand thread   (turn counter-clockwise to tighten).
Handedness = {
	Right = "Right",
	Left = "Left",
}

---@enum TipStyle
-- Thread crest tip style.
-- - "Sharp": full-height triangular crest (pointy). Prints crisp but can be fragile.
-- - "Cut":   flat/chamfered crest (truncated). Prints sturdier, easier to assemble.
TipStyle = {
	Sharp = "Sharp",
	Cut = "Cut",
}

--==========================================================================
-- ThreadSpec
--==========================================================================

---@class ThreadSpec
-- Specification shared by external (rod) and internal (cutter) threads.
-- Uses a **fit diameter** centered at the pitch radius so a single spec
-- can drive *both* male and female counterparts with predictable clearance.
-- Create with `ThreadSpec()` and set properties before use.
local ThreadSpec = {}

--- Create a new empty specification (all numeric fields default to 0).
---@return ThreadSpec
function ThreadSpec.new() end

--- Normalize/validate the spec in-place:
--- clamps negatives to 0, enforces minimum segment counts, etc.
---@return ThreadSpec self
function ThreadSpec:normalize() end

--- Diameter at the **pitch radius × 2**. For a pair (rod + nut) that should fit
--- each other, use the same `fitDiameter` in both specs.
---@type number
ThreadSpec.fitDiameter = 0

--- Axial advance per revolution (mm/rev).
---@type number
ThreadSpec.pitch = 0

--- Radial thread depth (mm) from pitch radius to crest/root midline.
---@type number
ThreadSpec.depth = 0

--- Extra radial slack for 3D-printing tolerances (mm).
--- Positive values loosen the fit (recommended: 0.10–0.30 for FDM).
---@type number
ThreadSpec.clearance = 0

--- Curve tessellation density along the helix (samples per turn).
--- Higher values make smoother threads at the cost of more triangles.
---@type integer
ThreadSpec.segmentsPerTurn = 0

--- Winding direction: `"right"` or `"left"` (string interface for convenience).
---@type '"right"'|'"left"'
ThreadSpec.handed = "right"

--- Crest tip style: `"sharp"` or `"cut"` (string interface).
---@type '"sharp"'|'"cut"'
ThreadSpec.tip = "sharp"

--- When `tip == "cut"`, specifies how much of the crest is truncated
--- (0.0 = nothing, 1.0 = full height). Typical values: 0.3–0.5.
---@type number
ThreadSpec.tipCutRatio = 0.0

--==========================================================================
-- Primitives
--==========================================================================

--- Create a solid **rod** (cylinder) with optional top/bottom end chamfers.
---
--- Chamfers are practical for printability and assembly: they ease starts and reduce elephant-foot.
---
--- @param diameter number  Outer diameter of the rod (mm).
--- @param length   number  Rod length (Z) in mm.
--- @param chamferBottom? boolean  If true, chamfer the Z=0 end. Default: false.
--- @param chamferTop?    boolean  If true, chamfer the Z=length end. Default: false.
--- @return Shape rod  The solid rod.
function rod(diameter, length, chamferBottom, chamferTop) end

--==========================================================================
-- Threaded rod
--==========================================================================

--- Make a **threaded rod** (external thread on a shaft) and return
--- both the solid and the resulting major diameter actually used.
---
--- Notes:
--- - The shaft length can be longer than the threaded length (e.g. partial threads).
--- - The resulting major diameter may include small adjustments for clearance
---   and tip style; use the second return value for reporting or mating parts.
---
--- @param totalLength  number        Full rod length (mm).
--- @param threadLength number        Axial length of threaded section (mm, 0..totalLength).
--- @param threadSpec   ThreadSpec    Thread specification (fit diameter, pitch, depth, etc.).
--- @return Shape threaded_rod        The resulting solid.
--- @return number actualMajorDiameter Major diameter applied (mm).
function threaded_rod(totalLength, threadLength, threadSpec) end

--==============================================================
-- PARAMETER TABLE (injected by engine)
--==============================================================

--- Global parameter table used by `param(name, default)`.
--- Values may be strings from CLI/GUI and are coerced by the engine when possible.
---@type table<string, any>
PARAMS = {}
