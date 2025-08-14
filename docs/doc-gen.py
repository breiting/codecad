import os
import re
from pathlib import Path

ROOT_DIR = Path.cwd()
SRC_DIR = ROOT_DIR / "lib"
DOCS_DIR = ROOT_DIR / "docs"
OUTPUT_FILE = DOCS_DIR / "LUA_LIBRARY.md"

# Regex, um --- Kommentare zu finden (LuaDoc / EmmyLua Stil)
COMMENT_BLOCK_RE = re.compile(r"^(---.*(?:\n---.*)*)", re.MULTILINE)


def extract_docblocks(lua_code):
    """Extrahiert --- Kommentarblöcke aus Lua-Code."""
    return COMMENT_BLOCK_RE.findall(lua_code)


def build_markdown(docs):
    """Baut die Markdown-Datei aus den gesammelten Kommentaren."""
    lines = ["# Lua Library Reference", ""]
    for file_path, blocks in docs.items():
        rel_path = os.path.relpath(file_path, ROOT_DIR)
        lines.append(f"## `{rel_path}`")
        lines.append("")
        if blocks:
            for block in blocks:
                # Entfernt führende ---
                clean_block = re.sub(r"^--- ?", "", block, flags=re.MULTILINE)
                lines.append(clean_block.strip())
                lines.append("")
        else:
            lines.append("_Keine Dokumentation gefunden._")
            lines.append("")
    return "\n".join(lines)


def main():
    DOCS_DIR.mkdir(exist_ok=True)
    docs = {}
    for lua_file in SRC_DIR.rglob("*.lua"):
        content = lua_file.read_text(encoding="utf-8")
        blocks = extract_docblocks(content)
        docs[lua_file] = blocks
    markdown = build_markdown(docs)
    OUTPUT_FILE.write_text(markdown, encoding="utf-8")
    print(f"Dokumentation geschrieben nach: {OUTPUT_FILE}")


if __name__ == "__main__":
    main()
