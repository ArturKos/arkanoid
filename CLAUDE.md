# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

A brick-breaker game in C++11 using Allegro 5. Single executable, no tests. The
workspace-level `../CLAUDE.md` covers cross-repo conventions; this file is
arkanoid-specific.

## Build & run

The `build/` directory is committed and holds the runtime assets
(`background.png`, `sounds/`, `levels/`). Read-only assets are resolved by
`paths.cpp`: `data_path()` checks the current directory first, then the
compiled-in `ARKANOID_DATA_DIR` (`<prefix>/share/arkanoid`, set in
CMakeLists.txt). So building and running from inside `build/` still works in
development, and an installed copy finds its assets under `/usr/share/arkanoid`:

```bash
cd build
cmake ..
make -j$(nproc)
./arkanoid          # run from build/ so the assets are found via CWD
```

`make install` (FHS layout via GNUInstallDirs) installs the binary, the data
dir, and `arkanoid.desktop`. Packagers pass `-DCMAKE_INSTALL_PREFIX=/usr` and
stage with `DESTDIR`.

Requires `liballegro5-dev` + `pkg-config` (see README for distro packages).
Compiler flags are `-Wall -Wextra -pedantic` (CMakeLists.txt). C++ standard is
effectively C++11; no test target exists.

High scores persist to `scores.dat` (binary `score_entry` records) under
`$XDG_DATA_HOME/arkanoid` (fallback `~/.local/share/arkanoid`), resolved by
`user_data_path()` in `paths.cpp`. Delete that file to reset.

## Architecture

Resolution-independent rendering is the core pattern. Everything draws at a fixed
`BOARD_WIDTH`×`BOARD_HEIGHT` (1024×768) logical size into an off-screen bitmap,
then `screen.cpp` scales that buffer to the real (resizable/fullscreen) display
with aspect-preserving letterboxing. The contract:
- `create_game_buffer()` / `destroy_game_buffer()` bookend the program.
- Every frame is wrapped in `begin_frame()` (targets the buffer, clears it) and
  `end_frame(display)` (scales+blits buffer to backbuffer, flips). Never draw
  directly to the display inside the loop; always draw at logical 1024×768.

`main.cpp` owns the program. Game state lives in **file-scope globals** (`score`,
`lives`, `poziom`, paddle `x`/`y`, power-up timers, `game_running`) plus two
global objects: `game_ball` and `game_tiles`. The single `while` loop in `main()`
drives three phases run in sequence: `run_intro()` → `prompt_name()` → the gameplay
loop, with `draw_high_scores()` shown on game-over (returns whether to restart via
`reset_game()`). Each of intro/name/scores runs its own internal event loop.

`game_objects.{h,cpp}` holds the object model:
- `ball` — position, velocity, speed, and a fixed-size trail buffer. Paddle-strike
  angle depends on which third of the paddle the ball hits.
- `tile` — HP (1–3), base color, beveled rendering with crack lines + HP dots.
- `tiles` — owns the fixed array of `tile*` (the 10×5 grid) plus `std::vector`s of
  `particle` and `powerup`, and a back-pointer to the `ball`. It is the hub for
  collision detection (`check_collisions` returns score gained and writes the
  shake timer), particle spawning, and power-up spawning/falling/collection.
  `game_over()` means the level is cleared (all tiles gone), triggering the next
  level — not loss of life.

`arkanoid.h` centralizes ALL tunable constants (sizes, speeds, counts, drop
chance, durations, scoring). Change gameplay values here, not inline.

## Conventions specific to this repo

- Timers and durations are counted in **frames**, not seconds (e.g.
  `POWERUP_DURATION 600`). The loop paces with `al_rest(0.01)` and gates paddle
  input on `al_get_time()`; there is no fixed-timestep integrator.
- Some identifiers are Polish — `rozm` (size), `poziom` (level), `okno` (window),
  `klawiatura` (keyboard), `czas` (time). Preserve them; do not anglicize.
- Allegro resources created with `al_create_*`/`al_load_*` are manually destroyed
  on every exit path in `main()`. When adding an early `return`, free the bitmap,
  font, buffer, and display first, matching the existing cleanup blocks.
- Power-up effects are applied in `main.cpp` after `collect_powerup()` returns a
  type constant; the wider/slow effects are reverted both on timer expiry and on
  level change / `reset_game()` — keep those two places in sync.
