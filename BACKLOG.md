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

Add work items here, in priority order. Example row format:

| # | Task | Status |
|---|------|--------|
| <!-- T1 | **Title** — concrete description: what to change, where, acceptance criteria. | ⬜ --> |

_(empty — add tasks before running the loop)_

---

## Out of scope — DO NOT implement

- Anything requiring a running game window / display / audio device to verify
  (the human play-tests those; the loop only builds).
- Speculative refactors or polish not listed above.
- A Windows/macOS port (project is Linux/Flatpak).
