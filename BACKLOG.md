# BACKLOG — finite implementation scope (source of truth for the architect agent)

This file is the **authoritative, finite checklist** the autonomous workflow's
architect agent (`scripts/agent_workflow.py`) works from. Each iteration the
architect:

1. Reads this file.
2. Verifies each `⬜` item against the actual code (it may already be done).
3. Marks it `✅` when fully satisfied (Doxygen + clean Linux build, `-Wall
   -Wextra -pedantic`, no new warnings; tests where a target exists).
   - **EXCEPTION — gameplay / visual / audio behaviour** (rendering, animation,
     sound, controls, "feel"): the loop runs headless and can't see/hear the
     game, so once the code is done + builds clean, mark it **`🔬`**
     (code-complete, awaiting the human's play-test), NOT `✅`.
4. Picks the next `⬜` item as the iteration's task. `🔬` items aren't re-picked.
5. When every item is `✅` or `🔬`, emits `all_implemented` and STOPS.

**Do NOT invent new scope** — only the items below are tasks. An empty task list
means there is nothing to do → the architect reports `all_implemented` immediately.

Status legend: `⬜` todo · `✅` done & verified · `🔬` code-complete, needs human play-test.

arkanoid is **Linux-only** (C++11 + Allegro 5, CMake). There is no Windows build.

---

## Tasks

In priority order (T1 first).

| # | Task | Status |
|---|------|--------|
| T1 | **Fancier falling power-ups** — the catchable drops look plain (a letter on a pill with a flat white glow/border). Redesign `tiles::draw_powerups()` in `game_objects.cpp` so each of the 5 types (WIDER/SLOW/LIFE/MULTI/FIRE) is visually distinct and lively, using Allegro primitives (no new image assets): (a) a **type-coloured** aura/glow (not white), (b) a small **drawn icon** per type instead of a bare letter (e.g. double-arrow for WIDE, hourglass/snail for SLOW, heart for LIFE, two small balls for MULTI, flame for FIRE), (c) a capsule with a subtle bevel/gradient + gentle pulse or spin and a short sparkle/after-trail as it falls. Keep the 1024×768 logical-coord rendering and `POWERUP_WIDTH/HEIGHT`. Keep collection logic unchanged. | 🔬 |
| T2 | **Laser power-up** — add a new power-up type (`POWERUP_LASER`) that, for its duration, lets the paddle fire lasers (e.g. SPACE) that travel up and destroy bricks on hit (respect tile HP). Add the projectile model, firing/cooldown, collision with `tiles`, draw + the HUD timer indicator (like WIDE/SLOW in `main.cpp`), and a drop case in the power-up switch. | 🔬 |
| T3 | **Catch / sticky paddle power-up** — classic Arkanoid "Catch": a power-up that makes the ball stick to the paddle on contact; the player releases it with a key (SPACE) or after a timeout. Handle reset/serve interaction and the multi-ball case. | 🔬 |
| T4 | **More levels + progression** — add several new hand-designed level layouts (the `levels/` data the game loads) and smooth progression between them (level-cleared transition / brief "LEVEL n" banner), looping or ending gracefully after the last. | ⬜ |
| T5 | **Game juice / feedback** — add screen-shake on brick break & ball loss, a short hit-flash/particle burst tuned per event, and richer audio cues (brick hit vs power-up collect vs life lost), without breaking the resolution-independent rendering. Make the intensity tasteful (configurable constants). | ⬜ |
| T6 | **Pause menu + basic settings** — ESC pauses gameplay into an overlay menu (Resume / Restart / Quit) and a small settings panel: master volume and fullscreen/windowed toggle, persisted alongside the existing scores data via `paths.cpp`. | ⬜ |

> All six are **gameplay/visual** features: the loop will implement + build them
> clean, but mark them `🔬` (you play-test) — only you can confirm they look/feel right.

---

## Out of scope — DO NOT implement

- Anything requiring a running game window / display / audio device to verify
  (the human play-tests those; the loop only builds).
- Speculative refactors or polish not listed above.
- A Windows/macOS port (project is Linux/Flatpak).
