#!/bin/sh

python3 scripts/extract_lua_api.py \
  --roots libs/lua/src  \
  --out-dlua lua/types/ccad/generated.d.lua \
  --out-json build/lua_api.json \
  --module-name ccad.generated
