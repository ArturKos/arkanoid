#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <cstdlib>
#include <cmath>
#include <vector>

#include "arkanoid.h"
#include "screen.h"
#include "intro.h"

// ============================================================
//  Pixel font for "ARKANOID" — each letter is 5 wide x 7 tall
// ============================================================

#define PF_W 5
#define PF_H 7
#define NUM_TITLE_LETTERS 8
#define CELL 13
#define LETTER_GAP 2       // cell-gap between letters
#define TITLE_Y 180

static const bool letter_data[NUM_TITLE_LETTERS][PF_H][PF_W] = {
  // A
  {{0,1,1,1,0},
   {1,0,0,0,1},
   {1,0,0,0,1},
   {1,1,1,1,1},
   {1,0,0,0,1},
   {1,0,0,0,1},
   {1,0,0,0,1}},
  // R
  {{1,1,1,1,0},
   {1,0,0,0,1},
   {1,0,0,0,1},
   {1,1,1,1,0},
   {1,0,1,0,0},
   {1,0,0,1,0},
   {1,0,0,0,1}},
  // K
  {{1,0,0,0,1},
   {1,0,0,1,0},
   {1,0,1,0,0},
   {1,1,0,0,0},
   {1,0,1,0,0},
   {1,0,0,1,0},
   {1,0,0,0,1}},
  // A
  {{0,1,1,1,0},
   {1,0,0,0,1},
   {1,0,0,0,1},
   {1,1,1,1,1},
   {1,0,0,0,1},
   {1,0,0,0,1},
   {1,0,0,0,1}},
  // N
  {{1,0,0,0,1},
   {1,1,0,0,1},
   {1,0,1,0,1},
   {1,0,0,1,1},
   {1,0,0,0,1},
   {1,0,0,0,1},
   {1,0,0,0,1}},
  // O
  {{0,1,1,1,0},
   {1,0,0,0,1},
   {1,0,0,0,1},
   {1,0,0,0,1},
   {1,0,0,0,1},
   {1,0,0,0,1},
   {0,1,1,1,0}},
  // I
  {{1,1,1,1,1},
   {0,0,1,0,0},
   {0,0,1,0,0},
   {0,0,1,0,0},
   {0,0,1,0,0},
   {0,0,1,0,0},
   {1,1,1,1,1}},
  // D
  {{1,1,1,1,0},
   {1,0,0,0,1},
   {1,0,0,0,1},
   {1,0,0,0,1},
   {1,0,0,0,1},
   {1,0,0,0,1},
   {1,1,1,1,0}}
};

// Rainbow colours per letter
static const unsigned char letter_rgb[NUM_TITLE_LETTERS][3] = {
  {255,  50,  50},   // A  — red
  {255, 150,  30},   // R  — orange
  {255, 255,  50},   // K  — yellow
  { 50, 255,  80},   // A  — green
  { 50, 220, 255},   // N  — cyan
  { 60, 100, 255},   // O  — blue
  {180,  60, 255},   // I  — purple
  {255,  60, 180},   // D  — pink
};

// ============================================================
//  Structures
// ============================================================

struct title_brick {
  float x, y;
  float target_x, target_y;
  float vy;
  bool  landed;
  float delay;          // seconds before brick starts falling
  unsigned char r, g, b;
  int   letter_idx;     // which letter it belongs to (for shimmer)
};

struct star {
  float x, y, speed;
  int brightness;
};

#define NUM_STARS 140
#define INTRO_BALL_R 12

// ============================================================
//  run_intro
// ============================================================

bool run_intro(ALLEGRO_DISPLAY *display, ALLEGRO_FONT *font) {
  (void)display;
  ALLEGRO_KEYBOARD_STATE kb;

  // --- Build title bricks ---------------------------------
  int total_cells = NUM_TITLE_LETTERS * PF_W +
                    (NUM_TITLE_LETTERS - 1) * LETTER_GAP;
  int title_w = total_cells * CELL;
  int base_x  = (BOARD_WIDTH - title_w) / 2;

  std::vector<title_brick> bricks;

  for (int L = 0; L < NUM_TITLE_LETTERS; L++) {
    int lx = base_x + L * (PF_W + LETTER_GAP) * CELL;
    for (int row = 0; row < PF_H; row++) {
      for (int col = 0; col < PF_W; col++) {
        if (!letter_data[L][row][col]) continue;
        title_brick tb;
        tb.target_x = (float)(lx + col * CELL);
        tb.target_y = (float)(TITLE_Y + row * CELL);
        tb.x        = tb.target_x;
        tb.y        = -(float)(CELL + rand() % 350);
        tb.vy       = 0;
        tb.landed   = false;
        tb.delay    = L * 0.15f + (float)(rand() % 60) / 600.0f;
        tb.r        = letter_rgb[L][0];
        tb.g        = letter_rgb[L][1];
        tb.b        = letter_rgb[L][2];
        tb.letter_idx = L;
        bricks.push_back(tb);
      }
    }
  }

  // --- Stars -----------------------------------------------
  star stars[NUM_STARS];
  for (int i = 0; i < NUM_STARS; i++) {
    stars[i].x = (float)(rand() % BOARD_WIDTH);
    stars[i].y = (float)(rand() % BOARD_HEIGHT);
    stars[i].speed = 0.2f + (float)(rand() % 100) / 70.0f;
    stars[i].brightness = 60 + rand() % 195;
  }

  // --- Bouncing ball ---------------------------------------
  float bx = BOARD_WIDTH * 0.3f, by = BOARD_HEIGHT * 0.6f;
  float bvx = 3.5f, bvy = 2.8f;

  // --- Trail for the ball ----------------------------------
  const int TRAIL_LEN = 18;
  float trail_x[TRAIL_LEN], trail_y[TRAIL_LEN];
  int trail_n = 0;

  // --- Bottom decorative bricks ----------------------------
  struct deco_brick { float x, y; unsigned char r, g, b; };
  std::vector<deco_brick> decos;
  {
    int cols = BOARD_WIDTH / CELL + 1;
    for (int c = 0; c < cols; c++) {
      deco_brick d;
      d.x = (float)(c * CELL);
      d.y = (float)(BOARD_HEIGHT - CELL);
      d.r = 40 + rand() % 60;
      d.g = 40 + rand() % 60;
      d.b = 50 + rand() % 80;
      decos.push_back(d);
    }
    for (int c = 0; c < cols; c++) {
      deco_brick d;
      d.x = (float)(c * CELL);
      d.y = (float)(BOARD_HEIGHT - 2 * CELL);
      d.r = 30 + rand() % 50;
      d.g = 30 + rand() % 50;
      d.b = 40 + rand() % 70;
      decos.push_back(d);
    }
  }

  double t0 = al_get_time();
  bool fs = false;
  bool f_prev = false;

  while (true) {
    al_get_keyboard_state(&kb);
    if (al_key_down(&kb, ALLEGRO_KEY_ESCAPE)) return false;

    // Fullscreen toggle
    bool f_now = al_key_down(&kb, ALLEGRO_KEY_F);
    if (f_now && !f_prev) {
      fs = !fs;
      al_set_display_flag(display, ALLEGRO_FULLSCREEN_WINDOW, fs);
    }
    f_prev = f_now;

    double elapsed = al_get_time() - t0;

    // SPACE to start (after controls are shown)
    if (elapsed > 2.8 && al_key_down(&kb, ALLEGRO_KEY_SPACE))
      return true;

    // ===== Update ==========================================

    // Stars
    for (int i = 0; i < NUM_STARS; i++) {
      stars[i].y += stars[i].speed;
      if (stars[i].y > BOARD_HEIGHT) {
        stars[i].y = 0;
        stars[i].x = (float)(rand() % BOARD_WIDTH);
      }
    }

    // Title bricks
    for (auto &b : bricks) {
      if (b.landed) continue;
      if (elapsed < (double)b.delay) continue;
      b.vy += 0.45f;          // gravity
      b.y  += b.vy;
      if (b.y >= b.target_y) {
        b.y  = b.target_y;
        b.vy = -b.vy * 0.35f; // bounce
        if (fabsf(b.vy) < 0.8f) {
          b.landed = true;
          b.y = b.target_y;
        }
      }
    }

    // Ball
    if (trail_n < TRAIL_LEN) {
      trail_x[trail_n] = bx;
      trail_y[trail_n] = by;
      trail_n++;
    } else {
      for (int i = 0; i < TRAIL_LEN - 1; i++) {
        trail_x[i] = trail_x[i + 1];
        trail_y[i] = trail_y[i + 1];
      }
      trail_x[TRAIL_LEN - 1] = bx;
      trail_y[TRAIL_LEN - 1] = by;
    }
    bx += bvx;
    by += bvy;
    if (bx - INTRO_BALL_R < 0 || bx + INTRO_BALL_R > BOARD_WIDTH)  bvx = -bvx;
    if (by - INTRO_BALL_R < 0 || by + INTRO_BALL_R > BOARD_HEIGHT) bvy = -bvy;

    // ===== Draw ============================================

    begin_frame();
    al_clear_to_color(al_map_rgb(4, 4, 18));

    // Stars
    for (int i = 0; i < NUM_STARS; i++) {
      int br = stars[i].brightness;
      float sz = stars[i].speed * 0.9f;
      unsigned char bc = (unsigned char)(br > 255 ? 255 : br);
      unsigned char bb = (unsigned char)(br + 30 > 255 ? 255 : br + 30);
      al_draw_filled_circle(stars[i].x, stars[i].y, sz,
                            al_map_rgba(bc, bc, bb, bc));
    }

    // Decorative bottom bricks
    for (auto &d : decos) {
      al_draw_filled_rectangle(d.x, d.y, d.x + CELL - 1, d.y + CELL - 1,
                               al_map_rgb(d.r, d.g, d.b));
      al_draw_line(d.x, d.y, d.x + CELL - 1, d.y,
                   al_map_rgba(255, 255, 255, 25), 1);
    }

    // Ball trail
    for (int i = 0; i < trail_n; i++) {
      float frac = (float)i / (float)trail_n;
      al_draw_filled_circle(trail_x[i], trail_y[i],
                            INTRO_BALL_R * frac * 0.6f,
                            al_map_rgba(255, 60, 10, (int)(frac * 60)));
    }

    // Ball glow + body
    al_draw_filled_circle(bx, by, INTRO_BALL_R + 6,
                          al_map_rgba(255, 80, 20, 25));
    al_draw_filled_circle(bx, by, INTRO_BALL_R,
                          al_map_rgba(220, 40, 0, 180));
    al_draw_filled_circle(bx - 3, by - 3, INTRO_BALL_R * 0.45f,
                          al_map_rgba(255, 180, 120, 100));

    // --- Title bricks (with shimmer) -----------------------
    for (auto &b : bricks) {
      if (!b.landed && elapsed < (double)b.delay) continue;

      // Shimmer: subtle brightness pulse per letter once landed
      float shimmer = 1.0f;
      if (b.landed) {
        float phase = (float)elapsed * 2.0f + b.letter_idx * 0.8f;
        shimmer = 0.85f + 0.15f * sinf(phase);
      }

      unsigned char dr = (unsigned char)(b.r * shimmer > 255 ? 255 : b.r * shimmer);
      unsigned char dg = (unsigned char)(b.g * shimmer > 255 ? 255 : b.g * shimmer);
      unsigned char db = (unsigned char)(b.b * shimmer > 255 ? 255 : b.b * shimmer);

      float x1 = b.x;
      float y1 = b.y;
      float x2 = b.x + CELL - 1;
      float y2 = b.y + CELL - 1;

      // Body
      al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(dr, dg, db));
      // Top-left highlight
      al_draw_line(x1, y1, x2, y1, al_map_rgba(255, 255, 255, 90), 1);
      al_draw_line(x1, y1, x1, y2, al_map_rgba(255, 255, 255, 60), 1);
      // Bottom-right shadow
      al_draw_line(x2, y1, x2, y2, al_map_rgba(0, 0, 0, 90), 1);
      al_draw_line(x1, y2, x2, y2, al_map_rgba(0, 0, 0, 90), 1);
    }

    // Glow behind title (once mostly landed)
    if (elapsed > 1.0) {
      float ga = (float)fmin(1.0, (elapsed - 1.0) / 1.0) * 0.35f;
      float glow_pulse = 0.8f + 0.2f * sinf((float)elapsed * 1.5f);
      int alpha = (int)(ga * glow_pulse * 255);
      float gx = (float)base_x - 20;
      float gy = (float)TITLE_Y - 15;
      float gw = (float)title_w + 40;
      float gh = (float)(PF_H * CELL) + 30;
      // Soft rectangles as fake glow
      for (int g = 0; g < 4; g++) {
        int a = alpha / (g + 2);
        float off = (float)(g * 8);
        al_draw_filled_rectangle(gx - off, gy - off, gx + gw + off,
                                 gy + gh + off,
                                 al_map_rgba(180, 140, 255, a));
      }
    }

    // Re-draw title bricks on top of glow
    for (auto &b : bricks) {
      if (!b.landed && elapsed < (double)b.delay) continue;

      float shimmer = 1.0f;
      if (b.landed) {
        float phase = (float)elapsed * 2.0f + b.letter_idx * 0.8f;
        shimmer = 0.85f + 0.15f * sinf(phase);
      }

      unsigned char dr = (unsigned char)(b.r * shimmer > 255 ? 255 : b.r * shimmer);
      unsigned char dg = (unsigned char)(b.g * shimmer > 255 ? 255 : b.g * shimmer);
      unsigned char db = (unsigned char)(b.b * shimmer > 255 ? 255 : b.b * shimmer);

      float x1 = b.x;
      float y1 = b.y;
      float x2 = b.x + CELL - 1;
      float y2 = b.y + CELL - 1;

      al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(dr, dg, db));
      al_draw_line(x1, y1, x2, y1, al_map_rgba(255, 255, 255, 90), 1);
      al_draw_line(x1, y1, x1, y2, al_map_rgba(255, 255, 255, 60), 1);
      al_draw_line(x2, y1, x2, y2, al_map_rgba(0, 0, 0, 90), 1);
      al_draw_line(x1, y2, x2, y2, al_map_rgba(0, 0, 0, 90), 1);
    }

    // --- Subtitle ------------------------------------------
    if (elapsed > 1.4) {
      float a = (float)fmin(1.0, (elapsed - 1.4) / 0.6);
      int alpha = (int)(a * 255);
      // Shadow
      al_draw_text(font, al_map_rgba(0, 0, 0, alpha / 2),
                   BOARD_WIDTH / 2 - 38, TITLE_Y + PF_H * CELL + 26, 0,
                   "by Artur Kos");
      al_draw_text(font, al_map_rgba(180, 180, 220, alpha),
                   BOARD_WIDTH / 2 - 40, TITLE_Y + PF_H * CELL + 24, 0,
                   "by Artur Kos");
    }

    // --- Controls info ---------------------------------------
    if (elapsed > 2.0) {
      float a = (float)fmin(1.0, (elapsed - 2.0) / 0.8);
      int alpha = (int)(a * 200);
      ALLEGRO_COLOR dim  = al_map_rgba(140, 140, 170, alpha);
      ALLEGRO_COLOR key  = al_map_rgba(220, 220, 255, alpha);
      float cy = TITLE_Y + PF_H * CELL + 60;
      float col1 = BOARD_WIDTH / 2.0f - 140;  // key column
      float col2 = BOARD_WIDTH / 2.0f - 60;   // description column

      al_draw_text(font, key, col1, cy,      0, "ARROWS");
      al_draw_text(font, dim, col2, cy,      0, "Move paddle");
      al_draw_text(font, key, col1, cy + 16, 0, "SPACE");
      al_draw_text(font, dim, col2, cy + 16, 0, "Launch ball");
      al_draw_text(font, key, col1, cy + 32, 0, "F");
      al_draw_text(font, dim, col2, cy + 32, 0, "Toggle fullscreen");
      al_draw_text(font, key, col1, cy + 48, 0, "ESC");
      al_draw_text(font, dim, col2, cy + 48, 0, "Quit");
    }

    // --- "Press SPACE to start" blinking -------------------
    if (elapsed > 2.8) {
      double blink = fmod(elapsed, 1.0);
      if (blink < 0.65) {
        float prompt_y = TITLE_Y + PF_H * CELL + 140;
        float cx = BOARD_WIDTH / 2.0f;
        const char *msg = "PRESS  SPACE  TO  START";

        // Dark backdrop bar
        al_draw_filled_rectangle(cx - 120, prompt_y - 6,
                                 cx + 120, prompt_y + 20,
                                 al_map_rgba(0, 0, 0, 160));
        al_draw_rectangle(cx - 120, prompt_y - 6,
                          cx + 120, prompt_y + 20,
                          al_map_rgba(255, 255, 100, 100), 1);

        // Bold text: draw at offsets for thickness
        ALLEGRO_COLOR col = al_map_rgb(255, 255, 100);
        al_draw_text(font, col, cx, prompt_y + 2, ALLEGRO_ALIGN_CENTRE, msg);
        al_draw_text(font, col, cx + 1, prompt_y + 2, ALLEGRO_ALIGN_CENTRE, msg);
        al_draw_text(font, col, cx, prompt_y + 3, ALLEGRO_ALIGN_CENTRE, msg);
        al_draw_text(font, col, cx + 1, prompt_y + 3, ALLEGRO_ALIGN_CENTRE, msg);
      }
    }

    // --- CRT scanline overlay ------------------------------
    for (int sy = 0; sy < BOARD_HEIGHT; sy += 3) {
      al_draw_line(0, (float)sy, (float)BOARD_WIDTH, (float)sy,
                   al_map_rgba(0, 0, 0, 35), 1);
    }

    // Vignette (dark corners)
    for (int v = 0; v < 60; v++) {
      int a = 60 - v;
      // Top
      al_draw_line(0, (float)v, (float)BOARD_WIDTH, (float)v,
                   al_map_rgba(0, 0, 0, a), 1);
      // Bottom
      al_draw_line(0, (float)(BOARD_HEIGHT - v),
                   (float)BOARD_WIDTH, (float)(BOARD_HEIGHT - v),
                   al_map_rgba(0, 0, 0, a), 1);
    }

    end_frame(display);
    al_rest(0.012);
  }
}
