"""
Extract global function names from .d.lua stubs for Lua LS,
so you can paste them into Lua.diagnostics.globals in .luarc.json.

Usage:
  python scripts/extract_globals.py types/core.d.lua
"""

import argparse
import glob
import json
import os
import re
import sys
from typing import Iterable, Set

# Matches:   function box(    | rotate_z  = function(
RE_FUNC_DEF = re.compile(r"^\s*function\s+([A-Za-z_]\w*)\s*\(")
RE_FUNC_ASSIGN = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*function\s*\(")

# Exkludiert "local " davor:
RE_LOCAL_FUNC = re.compile(r"^\s*local\s+function\s+")
RE_LOCAL_ASSIGN = re.compile(r"^\s*local\s+[A-Za-z_]\w*\s*=\s*function\s*\(")

# Modulmethoden (M.foo, ccad.util.box, etc.) ausschließen:
RE_DOTTED_LHS = re.compile(r"^\s*[\w\.]+\.[A-Za-z_]\w*\s*=\s*function\s*\(")
RE_DOTTED_DEF = re.compile(r"^\s*function\s+[\w\.]+\.[A-Za-z_]\w*\s*\(")


def extract_from_file(path: str) -> Set[str]:
    names: Set[str] = set()
    try:
        with open(path, "r", encoding="utf-8") as f:
            for line in f:
                if RE_LOCAL_FUNC.search(line) or RE_LOCAL_ASSIGN.search(line):
                    continue
                if RE_DOTTED_LHS.search(line) or RE_DOTTED_DEF.search(line):
                    continue

                m = RE_FUNC_DEF.match(line)
                if m:
                    names.add(m.group(1))
                    continue

                m = RE_FUNC_ASSIGN.match(line)
                if m:
                    names.add(m.group(1))
                    continue
    except Exception as e:
        print(f"[warn] could not read {path}: {e}", file=sys.stderr)
    return names


def collect(paths: Iterable[str]) -> Set[str]:
    out: Set[str] = set()
    for p in paths:
        # allow globs
        for path in glob.glob(p, recursive=True):
            if os.path.isdir(path):
                continue
            if not path.endswith(".lua"):
                continue
            out |= extract_from_file(path)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("paths", nargs="+", help="*.lua or glob paths to .d.lua stubs")
    ap.add_argument("--luarc", action="store_true", help="emit a .luarc.json snippet")
    ap.add_argument("--pretty", action="store_true", help="pretty-print JSON")
    args = ap.parse_args()

    names = sorted(collect(args.paths))
    if args.luarc:
        payload = {"Lua.diagnostics.globals": names}
    else:
        payload = names

    indent = 2 if args.pretty else None
    print(json.dumps(payload, indent=indent))


if __name__ == "__main__":
    main()
