# Lua Library Reference

## `lib/util/place.lua`

1D array along X; optionally replicated along Y

2D grid (X/Y) of a given shape builder

## `lib/util/transform.lua`

@module "util.transform"
Various helper functions

Moves the shape in x direction
@param shape Shape
@param dx number distance in mm

Moves the shape in y direction
@param shape Shape
@param dy number distance in mm

Moves the shape in z direction
@param shape Shape
@param dz number distance in mm

Rotates the shape around the x axis
@param shape Shape
@param angle number angle in degrees

Rotates the shape around the y axis
@param shape Shape
@param angle number angle in degrees

Rotates the shape around the z axis
@param shape Shape
@param angle number angle in degrees

## `lib/util/profile.lua`

Simple slot (rounded ends) in XY via difference helper

## `lib/util/frame.lua`

Push a copy of the current frame onto the stack.

Pop the current frame. (Keeps at least one frame.)

Temporarily enter a new scope (push → fn() → pop).

Translate current frame by (dx,dy,dz).

Rotate current frame (adds degrees).

Uniform scale for the current frame (multiplies).

Apply current frame to a shape and return the transformed shape.

Wrap core primitives to auto-apply the current frame.

Sugar: with_origin(dx,dy,dz, fn) – runs fn in a shifted local frame.

Sugar: with_frame({tx,ty,tz, rx,ry,rz, sc}, fn)

## `lib/util/box.lua`

Generates an open box with a certain size and wall thickness
@param L number @length (mm)
@param W number @width (mm)
@param H number @height (mm)
@param t number @wall thickness (mm)

## `lib/struct/wood.lua`

_Keine Dokumentation gefunden._

## `lib/struct/roof.lua`

_Keine Dokumentation gefunden._

## `lib/struct/deck.lua`

_Keine Dokumentation gefunden._

## `lib/mech/nut.lua`

_Keine Dokumentation gefunden._

## `lib/mech/gears.lua`

Spur gear with sensible defaults.

## `lib/mech/bolt.lua`

_Keine Dokumentation gefunden._

## `lib/mech/rod.lua`

Horizontal cylinder

Top-rounden rod holder with a optional hole in the middle (if diameter is missing or 0, no hole)
@param width number @total width in mm
@param height_bottom number @height of the rectangular shape
@param height_top number @height of the curved head (radius)
@param thickness number @thickness of extrusion
@param diameter number @optional if a rod hole should be applied

## `lib/mech/washer.lua`

_Keine Dokumentation gefunden._
