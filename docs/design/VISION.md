# Vision

**Adventure** is a first-person, melee-focused dark-fantasy action game in the spirit of **GRAVEN**
(Slipgate Ironworks' spiritual successor to Hexen), built on **raylib 5.5 + C++17**.

## The feeling we're chasing
- **Movement:** Quake/Doom-fluid — fast acceleration, air control, responsive, easy to learn, hard to
  master. Traversal itself should feel good before any combat.
- **Combat:** inspired by **Dark Messiah of Might & Magic** — directional melee with weight, a **kick**
  that knocks enemies back (into hazards, off ledges — environmental kills), and **shields**/blocking.
- **Look:** modern 3D geometry rendered *retro*. Full 3D world, but the frame is drawn at a low internal
  resolution and point-upscaled, with thick exponential fog, a dark gothic palette, baked/vertex lighting,
  and ordered dithering. Reference image: a first-person view holding a **torch** in one hand and a
  **sword** in the other, a gothic castle on a mountain, a medieval village below.

## Pillars
1. **Feel first.** The vertical slice exists to make one room, one weapon, one enemy *fun*. Breadth comes
   after the feel is proven.
2. **Data-driven, hot-reloadable.** Weapons, enemy AI, encounters, tuning, and UI live in sandboxed Lua so
   we iterate on feel and content without recompiling.
3. **Retro by construction, not by accident.** The pixelation/fog/dither is a deliberate pipeline, mirroring
   how GRAVEN fakes a 90s look on a modern engine.
4. **Instrumented from day one.** Every build can show memory, CPU, and per-section frame time (F3), so
   slow code is visible immediately.

## Scope arc
Single-player first: explorable areas, hidden secrets, scripted "arena" encounters, light RPG stats,
inventory, quests, interactive puzzles. Melee (staves, swords, daggers, maces, axes) + shields + ranged
(bows, crossbows) + spells. Low-fantasy bestiary (skeletons, orcs, goblins). Co-op multiplayer is a
long-horizon goal the architecture leaves room for, not an early target.

See [ROADMAP.md](ROADMAP.md) for milestones, [ARCHITECTURE.md](ARCHITECTURE.md) for how it's built,
and [COMBAT.md](COMBAT.md) for the combat design.
