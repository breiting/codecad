# TODOs

- [ ] Docker container
- [ ] Implement threading
- [ ] Lua snippets for CodeCAD
- [ ] Fix gears and threads module
- [ ] Select specific edges for chamfer and fillet

## DONE

- [x] Optimize `ccad parts add --name "<name>"` to `ccad add` (name is optional)
- [x] Implement BOM collector in C++
- [x] Project params should be populated to all parts (e.g. size of a table should be defined in project and populated to all parts)
- [x] Replace COSMA with PURE render engine
- [x] Camera should always use the bounding box as rotation center

## Further ideas

- Make tutorials
- Animations for parts
- `build` should be available from within the viewer (shortcut `b`)
- Let the viewer camera be controlled by Lua code (trajectories)
- Import STEP and/or DXF?
- Make CodeCAD as backend-module (core) which does the heavy lifting and returns geometry
- Simple web-based viewer using CodeCAD as backend
