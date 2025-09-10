# CodeCAD – Parametric CAD with Lua

**CodeCAD** ist ein leichtgewichtiges, skriptbares CAD für Maker, Ingenieur:innen und Forschende.
Du modellierst mit **Lua** (oder C++ im Kernel), siehst das Ergebnis live im **PURE** Viewer
und exportierst **STL/STEP** für 3D-Druck oder weitere CAD-Workflows.

## Warum CodeCAD?

- 💡 **Parametrisch**: Jede Abmessung ist eine Variable – ändere Zahlen, nicht Geometrie.
- 🧠 **Lua-first**: Eine klare, einfache Sprache statt proprietärer DSL.
- 🧩 **Erweiterbar**: C++-Kernel (OCCT), Lua-Bindings, modularer PURE-Viewer (GLFW/GL).
- 🧵 **Praktische Features**: Druckbare Gewinde, Rohradapter, Bezier/BSpline, Fillet/Chamfer mit Edge-Selector.
- 🛠️ **Open Source**: Baue dein Tooling selbst – oder nutze vorgefertigte Bausteine.

## Was kann ich bauen?

- Mechanische Teile (Bolzen, Muttern, Dosen mit Schraubdeckel, Halterungen, Zahnräder)
- Organische Formen (Vasen per Bezier/BSpline + Revolve)
- Adapter (Pipe-Transitions), gebogene Platten, etc.

## Schneller Eindruck (Lua)

```lua
-- Simple parametric box with rounded edges
local w, d, h = 100, 60, 20
local b = box(w, d, h)
b = fillet(b, 3)
emit(b)
```
