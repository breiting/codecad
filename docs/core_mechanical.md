# Mechanical

Mechanical features help you model real-world hardware quickly—smooth pipe transitions, printable rods, and robust screw threads. All measurements are in millimeters and the Z-axis is the default axial direction.

## ThreadSpec

`ThreadSpec` captures all parameters needed to generate compatible external (rod) and internal (cutter) threads. It’s based on a fit diameter centered at the pitch radius, so the same spec can drive both male and female parts with predictable clearance.

### Create & Normalize

```lua
local ts = ThreadSpec.new()
ts.fitDiameter = 20.0 -- at pitch radius × 2
ts.pitch = 2.0 -- mm per revolution
ts.depth = 0.9 -- radial depth from pitch radius
ts.clearance = 0.20 -- extra clearance for FDM
ts.segmentsPerTurn = 48 -- smoothness
ts.handed = "right" -- "right" | "left"
ts.tip = "cut" -- "sharp" | "cut"
ts.tipCutRatio = 0.4 -- 0.0..1.0 (only used when tip="cut")

ts:normalize() -- clamp/validate fields (recommended)
```

### Fields

- `fitDiameter` (number) — pitch radius × 2 used for both mating parts
- `pitch` (number) — axial advance per turn (mm/rev)
- `depth` (number) — radial thread depth from pitch radius to crest/root midline
- `clearance` (number) — extra radial slack; typical 0.10–0.30 for FDM printing
- `segmentsPerTurn` (integer) — tessellation density (more = smoother)
- `handed` ("right"|"left") — winding direction
- `tip` ("sharp"|"cut") — crest style
- `tipCutRatio` (number) — truncation fraction for "cut" tips, usually 0.3–0.5

### Practical guidance

- Use the same `fitDiameter` and pitch for mating rod/nut pairs.
- Increase clearance for tighter printers/materials; reduce for precision fits.
- `segmentsPerTurn` of 32–64 is a good balance for most parts.

## Pipe Adapter

```lua
pipe_adapter(rIn0, rOut0, rIn1, rOut1, length, steepness?)
```

Builds a smooth, revolved transition between two coaxial pipes. The adapter’s axis is Z; radii are taken at z=0 and z=length.

```lua
--8<-- "docs/assets/lua/pipe.lua"
```

<div class="stl-viewer"
     data-src="/assets/models/pipe.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

### Parameters

- rIn0, rOut0: inner/outer radius at z=0
- rIn1, rOut1: inner/outer radius at z=length
- length: axial distance along Z
- steepness?: smoothing factor [4..12] (defaults to a balanced value)
- **Returns** Shape — a watertight revolved solid

### Notes & Tips

- Use for reducers/expanders between hose or pipe standards.
- Keep wall thickness reasonable: rOutX - rInX should reflect manufacturable walls.
- Increase steepness for a longer, gentler transition (better flow, less turbulence).

## Rod

```lua
rod(diameter, length, chamferBottom?, chamferTop?)
```

Creates a cylindrical rod with optional end chamfers—handy as a starting shaft or as a carrier for threads.

```lua
-- 8 mm × 30 mm rod with a small chamfer on both ends
local r = rod(8, 30, true, true)
emit(r)
```

## rod(diameter, height)

Creates a simple cylindrical rod (alias for a cylinder, but semantically useful for mechanical parts).

```lua
--8<-- "docs/assets/lua/rod.lua"
```

<div class="stl-viewer"
     data-src="/assets/models/rod.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

### Parameters

- diameter: outer diameter (OD)
- length: along the Z-axis
- chamferBottom?, chamferTop?: booleans to ease starts and reduce elephant-foot
- **Returns** Shape — solid cylinder with optional chamfers

## Threaded Rod

```lua
threaded_rod(totalLength, threadLength, threadSpec)
```

Generates an external threaded shaft and returns both the solid and the actual major diameter applied after any adjustments for tip style and clearance.

```lua
-- M20-ish printable rod: 60 mm long, 45 mm threaded section
local ts = ThreadSpec.new()
ts.fitDiameter = 20.0
ts.pitch = 2.0
ts.depth = 0.9
ts.clearance = 0.20
ts.segmentsPerTurn = 48
ts.handed = "right"
ts.tip = "cut"
ts.tipCutRatio = 0.4
ts:normalize()

local tr, major = threaded_rod(60, 45, ts)
emit(tr)
-- You can report or design mating parts against `major` if needed.
```

### Parameters

- totalLength: full shaft length (mm)
- threadLength: axial length of the threaded section (0..totalLength)
- threadSpec: a validated ThreadSpec
- Returns
  - Shape — the threaded rod solid
  - number — actualMajorDiameter (mm)

## Patterns & Tips

- For partial threading, set threadLength < totalLength (e.g., leave a smooth shank).
- Use handed="left" for reverse threads in anti-rotation features.
- For small pitches or brittle materials, prefer tip="cut" with tipCutRatio≈0.4.
- Increase segmentsPerTurn for close-up aesthetics or metal printing.

## Putting It Together

Printable Reducer with Threaded Stub

```lua
-- 1) Reducer body
local reducer = pipe_adapter(8, 12, 6, 10, 25, 9) -- gentle transition

-- 2) Threaded stub: partial external thread on a rod
local ts = ThreadSpec.new()
ts.fitDiameter = 12.0
ts.pitch = 1.5
ts.depth = 0.6
ts.clearance = 0.18
ts.segmentsPerTurn = 48
ts.handed = "right"
ts.tip = "cut"
ts.tipCutRatio = 0.35
ts:normalize()

local stub, major = threaded_rod(20, 12, ts)

-- 3) Assemble
stub = center_xy(stub)
reducer = center_xy(reducer)
local assembly = union(reducer, translate(stub, 0, 0, -10))
emit(assembly)
```

## Gotchas

- Units & axes: All distances in mm; axial features are aligned to Z.
- Validate specs: Always call spec:normalize() to clamp and sanity-check inputs.
- Wall checks: Ensure rOut − rIn stays positive and realistic for printing or machining.
- Mating threads: Reuse the same fitDiameter/pitch for male/female; tune clearance for your process.
- Geometry complexity: Extremely tight bends, tiny pitches, or abrupt transitions may challenge meshing—raise segmentsPerTurn or adjust dimensions.

These mechanical tools give you parametric, fabrication-ready parts—smooth pipe transitions, robust rods, and threads that actually fit.
