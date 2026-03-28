# Arkanoid

A classic brick-breaker game built with **C++** and **Allegro 5**, featuring power-ups, particle effects, a ball trail system, persistent high scores, and a retro CRT-styled intro screen.

![C++](https://img.shields.io/badge/C%2B%2B-11-blue)
![Allegro](https://img.shields.io/badge/Allegro-5-green)
![CMake](https://img.shields.io/badge/CMake-3.2+-orange)
![Platform](https://img.shields.io/badge/Platform-Linux-lightgrey)

## Features

- **10x5 tile grid** with randomized colors and a multi-hit HP system (up to 3 HP per tile)
- **Beveled tile rendering** with highlight/shadow edges and visible crack lines as tiles take damage
- **HP indicator dots** displayed on multi-hit tiles so the player can gauge remaining durability
- **Three power-up types**: Wider paddle (W), Slow ball (S), and Extra life (+), each with a timed duration bar in the HUD
- **Particle explosion system** with gravity, color variation, and fade-out when tiles are destroyed
- **Ball trail effect** rendering a fading tail behind the ball as it moves
- **Screen shake** triggered on tile destruction for visual impact
- **Persistent high scores** saved to `scores.dat` in binary format, displaying the top 10 entries with animated row fade-in
- **Retro CRT intro screen** with falling pixel-art title bricks, rainbow shimmer, starfield background, bouncing ball, and decorative bottom bricks
- **Name input screen** with blinking cursor and CRT scanline overlay
- **Level progression**: clearing all tiles advances to the next level with a freshly randomized grid
- **Lives system** with 3 initial lives displayed as red circles in the HUD
- **Paddle angle control**: ball angle changes based on where it strikes the paddle (left third, center, right third)
- **Resizable window** with aspect-ratio-preserving scaling and fullscreen toggle (F key)

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| [Allegro 5](https://liballeg.org/) | >= 5.0 | Windowing, rendering, input, fonts, image loading, primitives |
| CMake | >= 3.2 | Build system |
| pkg-config | any | Allegro 5 detection |

### Installing dependencies

**Ubuntu / Debian:**
```bash
sudo apt-get install liballegro5-dev cmake pkg-config
```

**Arch Linux:**
```bash
sudo pacman -S allegro cmake pkgconf
```

**Fedora:**
```bash
sudo dnf install allegro5-devel cmake pkg-config
```

## Building

```bash
git clone https://github.com/ArturKos/arkanoid.git
cd arkanoid
cd build
cmake ..
make -j$(nproc)
```

## Running

Make sure `background.png` is present in the working directory, then run:

```bash
./arkanoid
```

## How to Play

1. **Intro screen** -- Watch the animated title or press SPACE to start
2. **Enter your name** -- Type your name (up to 16 characters) and press ENTER
3. **Launch the ball** -- Press SPACE to release the ball from the paddle
4. **Break all tiles** -- Move the paddle to bounce the ball and destroy the 10x5 grid
5. **Collect power-ups** -- Catch falling capsules with the paddle for temporary bonuses
6. **Advance levels** -- Clearing all tiles generates a new randomized grid
7. **Game over** -- Lose all 3 lives to see the high score table; press SPACE to restart

### Controls

| Action | Input |
|--------|-------|
| Move paddle left/right | Left / Right arrow |
| Move paddle up/down | Up / Down arrow (lower half of screen) |
| Launch ball | Space |
| Toggle fullscreen | F |
| Quit | Escape |

### Power-ups

| Type | Label | Effect | Duration |
|------|-------|--------|----------|
| Wider paddle | W (green) | Increases paddle width from 3x to 4.5x | 600 frames |
| Slow ball | S (blue) | Halves ball speed | 600 frames |
| Extra life | + (red) | Adds one life immediately | Instant |

Power-ups have a 25% drop chance when a tile is destroyed. Active power-up timers are displayed as progress bars in the top-left HUD.

## Scoring

| Event | Points |
|-------|--------|
| Hit a tile | +10 |
| Destroy a tile | +50 (bonus on top of hit) |

## Project Structure

```
arkanoid/
├── CMakeLists.txt          # Build configuration with Allegro 5 pkg-config
├── README.md               # This file
├── background.png          # Background image (required at runtime)
├── arkanoid.h              # Global constants (board size, speeds, power-up config)
├── main.cpp                # Game loop, input handling, HUD drawing, paddle rendering
├── game_objects.h          # Ball, tile, tiles container, particle, power-up declarations
├── game_objects.cpp         # Ball physics, tile HP/rendering, collision detection,
│                           #   particle system, power-up spawning and collection
├── intro.h                 # Intro screen declaration
├── intro.cpp               # Animated pixel-art title, starfield, bouncing ball intro
├── scores.h                # High score and name input declarations
├── scores.cpp              # Binary score file I/O, name prompt, game-over overlay
├── screen.h                # Off-screen buffer declarations
├── screen.cpp              # Resolution-independent rendering with aspect-ratio scaling
└── build/                  # CMake build directory
```

## License

This project is provided as-is for educational purposes.

---

**Author:** Artur Kos
