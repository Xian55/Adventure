---
name: adv-docs
description: Keep the project's documentation current after a code change — the subsystem CLAUDE.md files, the root CLAUDE.md, and docs/design. Use at the end of any feature or structural change (it's the final step of adv-feature), or whenever architecture, commands, conventions, or milestone status shift.
---

# adv-docs — keep docs and CLAUDE.md current

Docs rot fast. After a change, update what the change touched. Keep everything **lean** — CLAUDE.md files
are loaded into context every session, so they must stay short and scannable.

## What to update, and when
- **`src/<subsystem>/CLAUDE.md`** — you added/removed/renamed a file in that subsystem, changed its
  responsibility, added a gotcha, or changed how to extend it. This is the primary place detailed
  explanation lives (keep it out of code comments).
- **Root `CLAUDE.md`** — a command changed, a new subsystem/skill was added, a convention changed, or the
  milestone status moved.
- **`docs/design/`** — architecture or a design decision changed (`ARCHITECTURE.md`), a milestone
  completed/reordered (`ROADMAP.md`), combat/feel rules changed (`COMBAT.md`), a convention changed
  (`CONVENTIONS.md`), or perf baseline shifted (`DEVELOPMENT_LOOP.md`).
- **New subsystem?** Create `src/<name>/CLAUDE.md` (file table + rules + how-to-extend) and link it from the
  root layout section.

## Rules
- Lean and current beats complete and stale. Delete outdated lines; don't append forever.
- A CLAUDE.md is a *map*, not a manual: what each file owns, the gotchas, how to extend. Not a code dump.
- Cross-link: code comments point to the subsystem CLAUDE.md; CLAUDE.md points to `docs/design/`.
- If you changed a documented command or perf number, update the exact figure.

## Quick check
```bash
git diff --name-only        # which subsystems changed -> which CLAUDE.md need a look
```
