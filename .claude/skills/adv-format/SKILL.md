---
name: adv-format
description: Apply or check the Adventure C++ house style with clang-format. Use before committing C++ changes, or when CI's format gate fails. Formats src/tests/bench per .clang-format (tabs, Allman, ColumnLimit 0); never touches vendored deps/.
---

# adv-format — apply / check house style

Style is enforced by `.clang-format` (root) and gated in CI. clang-format **19.x** (VS ships one at
`C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\bin\clang-format.exe`; or `pip
install clang-format==19.1.5`). `deps/` is excluded via `deps/.clang-format` (DisableFormat).

## Apply (before committing)
```bash
CF="/c/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/bin/clang-format.exe"
files=$(git ls-files 'src/*.cpp' 'src/*.h' 'tests/*.cpp' 'bench/*.cpp' 'bench/*.h')
"$CF" -i $files
```

## Check (what CI runs — fails if anything isn't clean)
```bash
"$CF" --dry-run --Werror $(git ls-files 'src/*.cpp' 'src/*.h' 'tests/*.cpp' 'bench/*.cpp' 'bench/*.h')
```

## Style summary (see docs/design/CONVENTIONS.md for the full rule set)
- Tabs (width 4), Allman braces — but **lambda bodies stay inline**.
- `ColumnLimit: 0` — never auto-wrap; break long lines by hand.
- `m_` members, PascalCase types, `k`-constants, left pointers/refs (`int* p`, `int& r`).
- **One record per line** for multi-element vector/array/string-literal lists (trailing comma). clang-format
  preserves this layout but won't auto-explode a packed list — write it exploded.
- **Lean comments**: one line, the *why*. No walls — long explanation goes in the subsystem `CLAUDE.md`.
