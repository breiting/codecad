# CodeCAD in Practice

CodeCAD is more than just individual Lua scripts. A full project ties together parts, parameters, materials, and metadata into a reusable, adaptable design. Projects are stored in a human-readable JSON file (`project.json`) so that they can be shared, versioned, and modified easily.

This section explains how projects work and how they connect to real-world workflows such as 3D printing and construction.

## Project Structure

A CodeCAD project is described in a file called `project.json`. It contains the following main sections:

### meta

Basic information about the project.

```json
"meta": {
  "author": "Bernhard",
  "name": "Bracket",
  "units": "mm"
}
```

- name – Project name (any string).
- author – Creator’s name.
- units – The working units (usually "mm").

### materials

Defines available materials, usually by color for visualization.

```json
"materials": {
  "blue": { "color": "#5555ff" },
  "green": { "color": "#55ff55" },
  "oak": {"color": "#b58d63" }
}
```

- Each entry has a key (blue, green, oak) and a hex color.
- Useful for distinguishing parts visually in the viewer, or documenting intended material choice (plastic, wood, metal).

### params

Global parameters for your design. These can are getting populated to your individual Lua files. Which means that they act as global variables. The viewer picks up the project parameters and offers a UI to change the interactively.

```json
"params": {
   "Width": 30.0,
   "Distance": 70.0,
   "Height": 120.0
}
```

- Parameters are `numeric`, `boolean`, or `string` values.
- They act as variables that parts can use inside their Lua scripts.
- Change a parameter once → all dependent parts update automatically.

### parts

The heart of the project: a list of modeled components. The project should at least contain one part in order to render content.

```json
{
  "id": "plate",
  "name": "Upper Plate",
  "source": "parts/plate.lua",
  "material": "oak",
  "visible": true,
  "transform": {
    "translate": [45.0, 0.0, 0.0],
    "rotate": [0.0, 0.0, 0.0],
    "scale": 1.0
  }
}
```

- id – Unique identifier for the part.
- name – Human-readable name.
- source – Relative path to the Lua script that generates this part.
- material – Which material (defined in materials) to use.
- visible – Whether the part is shown in the viewer.
- transform – Local transformations of the part
  - translate: Move part in X/Y/Z.
  - rotate: Rotate in degrees.
  - scale: Uniform scaling factor.

### version

Specifies the project file format version (currently 1).

## From Idea to Physical Part

A project can contain multiple parts, each with different material and role.
For example:

• Blue part = a steel mounting plate.
• Green part = a wooden frame element.
• Red part = a 3D-printed bolt.

In the viewer, you can see how these parts interact, move them into place, and check clearances — all while keeping them parameterized for later adjustments.

## Workflow in Practice

1. Create your project directory and initialize the structure.
1. Add and design your parts as Lua scripts (`parts/plate.lua`, `parts/bolt.lua`, ...).
1. View live in the viewer to check fit and proportions.
1. Export to 3D printing with `ccad build`
1. (Optional) Export to construction/manufacturing: add `save_step(part, "part.step")` at the end of your Lua file.

## Keeping Designs Adaptable

The power of CodeCAD lies in its parametric design:

- Store and define important dimensions as params (`local width = param("Width", 20)`).
- Reference these parameters in your Lua parts.
- When requirements change (a thicker plate, a longer distance), just edit the number in your Lua file or directly in your `project.json`.
- All parts update automatically — no manual rework needed.
