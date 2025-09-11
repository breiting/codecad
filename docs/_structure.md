📖 CodeCAD Documentation Structure

1. Introduction
   • What is CodeCAD?
   • Why code-based CAD?
   • Key benefits: parametric design, adaptability, reproducibility.
   • Comparison to GUI-based CAD tools.

2. Getting Started
   • Installation (CLI, viewer, lab).
   • First run: generating a simple model.
   • Basic workflow: write Lua → preview → export.

3. Design Principles
   • From idea to part: Observe → Think → Sketch → Measure → Code.
   • Parametric modeling mindset.
   • When to sketch on paper vs. when to go straight into CodeCAD.
   • Iteration and adaptability (change a parameter → regenerate).

4. CodeCAD in Practice
   • CodeCAD project structure (explain the project.json file) can have material, parts, meta ...
   • From idea to physical part: this should contain a real world example of two parts with different material
   • View with live viewer (link to viewer section)
   • Export to 3D printing (STL) with `ccad build`
   • Export to construction/manufacturing (STEP). `save_step`
   • Keeping designs adaptable and reusable.

5. Core Functionality

Grouped by functional “packages,” with examples for each function:
• Primitives (box, cylinder, sphere, …)
• Curves & Sketch (poly_xy, rect, profile_xz, …)
• Transforms (translate, rotate_x, scale, …)
• Booleans / Operations (union, difference, intersection)
• Features (fillet_all, chamfer_all, threaded_rod, …)
• Mechanical helpers (pipe_adapter, wedge, …)
• Measure & Utilities (bbox, dimensions, tolerances)
• IO / Workflow (emit, save_stl, save_step)
• Selection & Advanced usage

👉 Each function should include: short explanation + Lua example + small rendering screenshot.

6. Architecture
   • Kernel (OpenCascade foundations).
   • Lua layer (scripting and API).
   • CodeCAD CLI (ccad) – generating and exporting models.
   • CodeCAD Lab – interactive environment.

7. Viewer
   • Interface overview.
   • Shortcuts & navigation.
   • Features (layer visibility, measurement tools, section cuts, etc.).
   • Export and integration into workflows.

8. Tutorials & Examples
   • Beginner: making a parametric box with a hole.
   • Intermediate: designing a gear with threaded_rod and Boolean cuts.
   • Advanced: assembly of multiple parametric parts.
   • Real-world workflows:
   • 3D-printable brackets.
   • Furniture joints.
   • Mechanical adapters.

9. API Reference
   • Full Lua API, grouped by package.
   • Function signatures, arguments, and defaults.
   • Links to example snippets.

10. CodeCAD Library
    • The additional library

11. Cheatsheet
    • One-page overview of primitives, transforms, operations.
    • Exportable as PDF for quick desk reference.

12. Glossary (optional but useful)
    • Engineering & CAD terminology (your German-English table fits here).
    • Quick definitions for newcomers.
