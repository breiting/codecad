#!/usr/bin/env python3
"""
extract_lua_api.py
Scan C++ sol2 bindings and Lua files to generate Lua LSP stubs (.d.lua) and/or JSON.

Finds:
  - lua.set_function("name", ...)
  - state.set_function("name", ...)
  - *.new_usertype<T>("ExposedName", "field", &T::field, "method", &T::Method, ..., sol::constructors<...>())
  - Lua functions in .lua: function M.foo(...), function foo(...)

Limitations:
  - Won't expand sol::overload signatures (names are extracted, args become `...`).
  - new_usertype key/value list is parsed heuristically; complex expressions may be skipped.
  - For best results, keep bindings simple or add comment hints like:
      // @doc:method param1:number param2:string -> boolean
"""

import argparse
import json
import re
from pathlib import Path
from collections import defaultdict

# -------- regexes --------
SET_FUNC_RE = re.compile(
    r"""(?P<obj>\b(?:lua|L|state|.*\bstate_view|.*\blua))\s*\.\s*set_function\s*\(\s*  # lua.set_function(
        ["'](?P<name>[^"']+)["']""",
    re.VERBOSE,
)

NEW_USERTYPE_RE = re.compile(
    r"""new_usertype\s*<\s*(?P<cpp>[^>]+)\s*>\s*\(\s*["'](?P<lname>[^"']+)["']\s*,(?P<body>.*?)\)\s*;""",
    re.VERBOSE | re.DOTALL,
)

# Matches pairs: "key", something, "key2", something2, ...
UT_PAIR_RE = re.compile(r'["\'](?P<key>[^"\']+)["\']\s*,', re.DOTALL)

CONSTRUCTORS_RE = re.compile(r"sol\s*::\s*constructors\s*<([^>]+)>")

# Lua files
LUA_FUNC_RE = re.compile(
    r"(?m)^\s*function\s+(?:([A-Za-z0-9_]+)\.)?([A-Za-z0-9_]+)\s*\("
)


def read_text(p: Path) -> str:
    try:
        return p.read_text(encoding="utf-8", errors="ignore")
    except Exception:
        return ""


def harvest_cpp(text: str, results: dict):
    # globals
    for m in SET_FUNC_RE.finditer(text):
        name = m.group("name").strip()
        results["globals"].add(name)

    # usertypes
    for m in NEW_USERTYPE_RE.finditer(text):
        exposed = m.group("lname").strip()
        body = m.group("body")

        # quick pass over key list
        keys = [mm.group("key") for mm in UT_PAIR_RE.finditer(body)]

        # constructors?
        ctors = []
        for c in CONSTRUCTORS_RE.finditer(body):
            sig = c.group(1).strip()
            ctors.append(sig)

        ut = results["usertypes"].setdefault(
            exposed,
            {
                "fields": set(),
                "methods": set(),
                "constructors": ctors,
            },
        )
        # very rough heuristic: names that look like verbs -> method; others -> field
        for k in keys:
            if re.match(
                r"^(get|set|add|remove|to|from|on|with|make|create|build|apply|update|clear|push|pop)\b",
                k,
                re.I,
            ):
                ut["methods"].add(k)
            else:
                # names like "left", "xy", "segments" will land here
                ut["fields"].add(k)


def harvest_lua(text: str, results: dict):
    for m in LUA_FUNC_RE.finditer(text):
        mod, fname = m.groups()
        if mod is None:
            # top-level function
            results["lua_globals"].add(fname)
        else:
            # module function
            modname = mod
            results["lua_modules"][modname].add(fname)


def scan_roots(roots):
    results = {
        "globals": set(),
        "usertypes": {},  # name -> {fields:set, methods:set, constructors:[...]}
        "lua_globals": set(),
        "lua_modules": defaultdict(set),
    }
    for root in roots:
        root = Path(root)
        for p in root.rglob("*"):
            if p.suffix in (".hpp", ".hh", ".h", ".cpp", ".cc", ".cxx"):
                harvest_cpp(read_text(p), results)
            elif p.suffix == ".lua":
                harvest_lua(read_text(p), results)
    return results


# ---- emitters ----
def emit_dlua(results, module_name: str) -> str:
    lines = []
    lines.append("---@meta")
    if module_name:
        lines.append(f'---@module "{module_name}"')
    lines.append("")

    # C++-exposed globals
    if results["globals"]:
        lines.append("-- C++ bindings (globals)")
        for name in sorted(results["globals"]):
            lines.append(f"---@return any")
            lines.append(f"function {name}(...) end")
            lines.append("")
    # Lua globals
    if results["lua_globals"]:
        lines.append("-- Lua helpers (globals)")
        for name in sorted(results["lua_globals"]):
            lines.append(f"---@return any")
            lines.append(f"function {name}(...) end")
            lines.append("")

    # usertypes
    for ut_name, data in sorted(results["usertypes"].items()):
        lines.append(f"---@class {ut_name}")
        lines.append(f"local {ut_name} = {{}}")
        lines.append("")
        # fields
        for f in sorted(data["fields"]):
            lines.append(f"---@field {f} any")
        if data["fields"]:
            lines.append("")
        # constructors (hint only)
        if data["constructors"]:
            for sig in data["constructors"]:
                lines.append(f"-- ctor: {sig}")
            lines.append("")
        # methods
        for m in sorted(data["methods"]):
            lines.append(f"---@param ... any")
            lines.append(f"---@return any")
            lines.append(f"function {ut_name}:{m}(...) end")
            lines.append("")
        lines.append(f"return {ut_name}")
        lines.append("")
    # module tables from Lua
    for mod, fns in sorted(results["lua_modules"].items()):
        lines.append(f"---@class {mod}")
        lines.append(f"local {mod} = {{}}")
        lines.append("")
        for fn in sorted(fns):
            lines.append(f"---@param ... any")
            lines.append(f"---@return any")
            lines.append(f"function {mod}.{fn}(...) end")
            lines.append("")
        lines.append(f"return {mod}")
        lines.append("")
    return "\n".join(lines).rstrip() + "\n"


def emit_json(results) -> str:
    def set_to_sorted_list(s):
        return sorted(list(s))

    out = {
        "globals": set_to_sorted_list(results["globals"]),
        "lua_globals": set_to_sorted_list(results["lua_globals"]),
        "lua_modules": {
            k: set_to_sorted_list(v) for k, v in results["lua_modules"].items()
        },
        "usertypes": {
            name: {
                "fields": set_to_sorted_list(data["fields"]),
                "methods": set_to_sorted_list(data["methods"]),
                "constructors": data["constructors"],
            }
            for name, data in results["usertypes"].items()
        },
    }
    return json.dumps(out, indent=2)


def main():
    ap = argparse.ArgumentParser(
        description="Extract Lua API (sol2 + Lua) and emit .d.lua / JSON."
    )
    ap.add_argument(
        "--roots",
        nargs="+",
        required=True,
        help="Folders to scan (e.g. libs/lua/src libs/kernel/src lib/)",
    )
    ap.add_argument("--out-dlua", help="Output .d.lua path")
    ap.add_argument("--out-json", help="Output JSON path")
    ap.add_argument(
        "--module-name",
        default="ccad.generated",
        help="Module name to stamp into .d.lua (default: ccad.generated)",
    )
    args = ap.parse_args()

    results = scan_roots(args.roots)

    if args.out_dlua:
        Path(args.out_dlua).write_text(
            emit_dlua(results, args.module_name), encoding="utf-8"
        )
        print(f"Wrote {args.out_dlua}")
    if args.out_json:
        Path(args.out_json).write_text(emit_json(results), encoding="utf-8")
        print(f"Wrote {args.out_json}")

    if not args.out_dlua and not args.out_json:
        # print a quick summary to stdout
        print(emit_json(results))


if __name__ == "__main__":
    main()
