#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

#include "arkanoid.h"
#include "screen.h"
#include "scores.h"

// ============================================================
//  File I/O
// ============================================================

std::vector<score_entry> load_scores() {
  std::vector<score_entry> entries;
  FILE *f = fopen(SCORES_FILE, "rb");
  if (!f) return entries;

  score_entry e;
  while (fread(&e, sizeof(score_entry), 1, f) == 1) {
    entries.push_back(e);
  }
  fclose(f);

  std::sort(entries.begin(), entries.end(),
            [](const score_entry &a, const score_entry &b) {
              return a.score > b.score;
            });

  if ((int)entries.size() > MAX_HIGH_SCORES)
    entries.resize(MAX_HIGH_SCORES);

  return entries;
}

void save_score(const char *name, int sc, int level) {
  std::vector<score_entry> entries = load_scores();

  score_entry ne;
  memset(&ne, 0, sizeof(ne));
  strncpy(ne.name, name, MAX_NAME_LENGTH);
  ne.name[MAX_NAME_LENGTH] = '\0';
  ne.score = sc;
  ne.level = level;
  entries.push_back(ne);

  std::sort(entries.begin(), entries.end(),
            [](const score_entry &a, const score_entry &b) {
              return a.score > b.score;
            });

  if ((int)entries.size() > MAX_HIGH_SCORES)
    entries.resize(MAX_HIGH_SCORES);

  FILE *f = fopen(SCORES_FILE, "wb");
  if (!f) return;
  for (auto &e : entries)
    fwrite(&e, sizeof(score_entry), 1, f);
  fclose(f);
}

// ============================================================
//  Name input prompt
// ============================================================

bool prompt_name(ALLEGRO_DISPLAY *display, ALLEGRO_FONT *font, char *out_name) {
  char name[MAX_NAME_LENGTH + 1];
  memset(name, 0, sizeof(name));
  int len = 0;
  bool done = false;
  bool prev_key[ALLEGRO_KEY_MAX];
  memset(prev_key, 0, sizeof(prev_key));

  // Drain any held keys
  al_rest(0.15);

  ALLEGRO_KEYBOARD_STATE kb;
  double t0 = al_get_time();

  while (!done) {
    al_get_keyboard_state(&kb);

    if (al_key_down(&kb, ALLEGRO_KEY_ESCAPE)) return false;

    // Handle ENTER
    if (al_key_down(&kb, ALLEGRO_KEY_ENTER) && !prev_key[ALLEGRO_KEY_ENTER]) {
      if (len > 0) done = true;
    }

    // Handle BACKSPACE
    if (al_key_down(&kb, ALLEGRO_KEY_BACKSPACE) &&
        !prev_key[ALLEGRO_KEY_BACKSPACE]) {
      if (len > 0) { len--; name[len] = '\0'; }
    }

    // Handle letter/digit keys
    if (len < MAX_NAME_LENGTH) {
      for (int k = ALLEGRO_KEY_A; k <= ALLEGRO_KEY_Z; k++) {
        if (al_key_down(&kb, k) && !prev_key[k]) {
          name[len++] = 'A' + (k - ALLEGRO_KEY_A);
          name[len] = '\0';
        }
      }
      for (int k = ALLEGRO_KEY_0; k <= ALLEGRO_KEY_9; k++) {
        if (al_key_down(&kb, k) && !prev_key[k]) {
          name[len++] = '0' + (k - ALLEGRO_KEY_0);
          name[len] = '\0';
        }
      }
      if (al_key_down(&kb, ALLEGRO_KEY_SPACE) &&
          !prev_key[ALLEGRO_KEY_SPACE] && len > 0) {
        name[len++] = ' ';
        name[len] = '\0';
      }
    }

    // Store previous key state
    for (int k = 0; k < ALLEGRO_KEY_MAX; k++)
      prev_key[k] = al_key_down(&kb, k);

    double elapsed = al_get_time() - t0;

    // --- Draw ---
    begin_frame();
    al_clear_to_color(al_map_rgb(4, 4, 18));

    // Scanlines
    for (int sy = 0; sy < BOARD_HEIGHT; sy += 3)
      al_draw_line(0, (float)sy, (float)BOARD_WIDTH, (float)sy,
                   al_map_rgba(0, 0, 0, 30), 1);

    float cx = BOARD_WIDTH / 2.0f;
    float cy = BOARD_HEIGHT / 2.0f - 40;

    // Title
    al_draw_text(font, al_map_rgb(255, 255, 100), cx, cy - 60,
                 ALLEGRO_ALIGN_CENTRE, "ENTER YOUR NAME");
    al_draw_text(font, al_map_rgb(255, 255, 100), cx + 1, cy - 59,
                 ALLEGRO_ALIGN_CENTRE, "ENTER YOUR NAME");

    // Input box
    float box_w = 200;
    float box_h = 24;
    float bx1 = cx - box_w / 2;
    float by1 = cy - box_h / 2;
    float bx2 = cx + box_w / 2;
    float by2 = cy + box_h / 2;

    al_draw_filled_rectangle(bx1, by1, bx2, by2, al_map_rgba(10, 10, 40, 220));
    al_draw_rectangle(bx1, by1, bx2, by2, al_map_rgba(100, 120, 220, 200), 1.5f);

    // Name text
    al_draw_text(font, al_map_rgb(255, 255, 255), cx, cy - 4,
                 ALLEGRO_ALIGN_CENTRE, name);

    // Blinking cursor
    if (fmod(elapsed, 0.8) < 0.5) {
      float cursor_x = cx + (float)len * 4.0f;
      // Approximate: builtin font is 8px wide, centered text starts at cx - len*4
      al_draw_line(cursor_x, by1 + 4, cursor_x, by2 - 4,
                   al_map_rgb(255, 255, 255), 1.5f);
    }

    // Hint
    al_draw_text(font, al_map_rgba(150, 150, 170, 200), cx, by2 + 16,
                 ALLEGRO_ALIGN_CENTRE, "Press ENTER to confirm");

    end_frame(display);
    al_rest(0.02);
  }

  strncpy(out_name, name, MAX_NAME_LENGTH + 1);
  return true;
}

// ============================================================
//  High scores display (game over overlay)
// ============================================================

bool draw_high_scores(ALLEGRO_DISPLAY *display, ALLEGRO_FONT *font,
                      const char *player_name, int player_score,
                      int player_level) {
  // Save the score first
  save_score(player_name, player_score, player_level);
  std::vector<score_entry> entries = load_scores();

  ALLEGRO_KEYBOARD_STATE kb;
  bool f_prev = false;
  bool fs = (al_get_display_flags(display) & ALLEGRO_FULLSCREEN_WINDOW) != 0;

  // Drain keys
  al_rest(0.2);

  double t0 = al_get_time();

  while (true) {
    al_get_keyboard_state(&kb);
    if (al_key_down(&kb, ALLEGRO_KEY_ESCAPE)) return false;
    if (al_key_down(&kb, ALLEGRO_KEY_SPACE)) return true;

    // Fullscreen toggle
    bool f_now = al_key_down(&kb, ALLEGRO_KEY_F);
    if (f_now && !f_prev) {
      fs = !fs;
      al_set_display_flag(display, ALLEGRO_FULLSCREEN_WINDOW, fs);
    }
    f_prev = f_now;

    double elapsed = al_get_time() - t0;

    begin_frame();
    al_clear_to_color(al_map_rgb(4, 4, 18));

    // Scanlines
    for (int sy = 0; sy < BOARD_HEIGHT; sy += 3)
      al_draw_line(0, (float)sy, (float)BOARD_WIDTH, (float)sy,
                   al_map_rgba(0, 0, 0, 30), 1);

    float cx = BOARD_WIDTH / 2.0f;

    // "GAME OVER" title
    ALLEGRO_COLOR red = al_map_rgb(255, 60, 60);
    al_draw_text(font, red, cx, 80, ALLEGRO_ALIGN_CENTRE, "GAME  OVER");
    al_draw_text(font, red, cx + 1, 81, ALLEGRO_ALIGN_CENTRE, "GAME  OVER");
    al_draw_text(font, red, cx, 81, ALLEGRO_ALIGN_CENTRE, "GAME  OVER");
    al_draw_text(font, red, cx + 1, 80, ALLEGRO_ALIGN_CENTRE, "GAME  OVER");

    // Player result
    char buf[128];
    snprintf(buf, sizeof(buf), "%s  -  Score: %d  Level: %d",
             player_name, player_score, player_level);
    al_draw_text(font, al_map_rgb(255, 255, 100), cx, 120,
                 ALLEGRO_ALIGN_CENTRE, buf);

    // High scores header
    float table_top = 170;

    // Header bar
    al_draw_filled_rectangle(cx - 220, table_top - 2,
                             cx + 220, table_top + 14,
                             al_map_rgba(60, 60, 120, 180));
    al_draw_textf(font, al_map_rgb(200, 200, 255), cx - 200, table_top,
                  0, "RANK");
    al_draw_text(font, al_map_rgb(200, 200, 255), cx - 120, table_top,
                 0, "NAME");
    al_draw_text(font, al_map_rgb(200, 200, 255), cx + 80, table_top,
                 0, "SCORE");
    al_draw_text(font, al_map_rgb(200, 200, 255), cx + 160, table_top,
                 0, "LVL");

    // Entries
    bool player_highlighted = false;
    for (int i = 0; i < (int)entries.size(); i++) {
      float ey = table_top + 22 + i * 20;

      // Fade-in per row
      float row_delay = 0.3f + i * 0.1f;
      float alpha = (float)fmin(1.0, (elapsed - row_delay) / 0.3);
      if (alpha <= 0) continue;

      // Highlight current player's entry
      bool is_player = (!player_highlighted &&
                        entries[i].score == player_score &&
                        strcmp(entries[i].name, player_name) == 0);
      if (is_player) player_highlighted = true;

      ALLEGRO_COLOR row_col;
      if (is_player) {
        // Pulsing border-only highlight — no filled rectangle
        float pulse = 0.6f + 0.4f * sinf((float)elapsed * 4.0f);
        ALLEGRO_COLOR border = al_map_rgba(255, 255, 100, (int)(pulse * 200));
        al_draw_rectangle(cx - 222, ey - 3, cx + 222, ey + 15, border, 2.0f);
        // Arrow marker on the left
        al_draw_filled_triangle(cx - 230, ey + 6,
                                cx - 222, ey + 1,
                                cx - 222, ey + 11, border);
        row_col = al_map_rgba(255, 255, 100, (int)(alpha * 255));
      } else {
        row_col = al_map_rgba(200, 200, 220, (int)(alpha * 255));
      }

      // Rank
      snprintf(buf, sizeof(buf), "%2d.", i + 1);
      al_draw_text(font, row_col, cx - 200, ey, 0, buf);
      // Name
      al_draw_text(font, row_col, cx - 120, ey, 0, entries[i].name);
      // Score
      snprintf(buf, sizeof(buf), "%d", entries[i].score);
      al_draw_text(font, row_col, cx + 80, ey, 0, buf);
      // Level
      snprintf(buf, sizeof(buf), "%d", entries[i].level);
      al_draw_text(font, row_col, cx + 160, ey, 0, buf);

      // Separator line
      al_draw_line(cx - 220, ey + 16, cx + 220, ey + 16,
                   al_map_rgba(60, 60, 100, (int)(alpha * 80)), 1);
    }

    // If no entries
    if (entries.empty()) {
      al_draw_text(font, al_map_rgb(150, 150, 170), cx, table_top + 30,
                   ALLEGRO_ALIGN_CENTRE, "No scores yet!");
    }

    // Blinking prompt
    if (fmod(elapsed, 1.0) < 0.65) {
      float prompt_y = BOARD_HEIGHT - 80;
      al_draw_filled_rectangle(cx - 120, prompt_y - 4,
                               cx + 120, prompt_y + 16,
                               al_map_rgba(0, 0, 0, 160));
      al_draw_rectangle(cx - 120, prompt_y - 4,
                        cx + 120, prompt_y + 16,
                        al_map_rgba(255, 255, 100, 100), 1);
      ALLEGRO_COLOR col = al_map_rgb(255, 255, 100);
      al_draw_text(font, col, cx, prompt_y, ALLEGRO_ALIGN_CENTRE,
                   "PRESS  SPACE  TO  RESTART");
      al_draw_text(font, col, cx + 1, prompt_y, ALLEGRO_ALIGN_CENTRE,
                   "PRESS  SPACE  TO  RESTART");
      al_draw_text(font, col, cx, prompt_y + 1, ALLEGRO_ALIGN_CENTRE,
                   "PRESS  SPACE  TO  RESTART");
      al_draw_text(font, col, cx + 1, prompt_y + 1, ALLEGRO_ALIGN_CENTRE,
                   "PRESS  SPACE  TO  RESTART");
    }

    // Vignette
    for (int v = 0; v < 40; v++) {
      int a = 40 - v;
      al_draw_line(0, (float)v, (float)BOARD_WIDTH, (float)v,
                   al_map_rgba(0, 0, 0, a), 1);
      al_draw_line(0, (float)(BOARD_HEIGHT - v),
                   (float)BOARD_WIDTH, (float)(BOARD_HEIGHT - v),
                   al_map_rgba(0, 0, 0, a), 1);
    }

    end_frame(display);
    al_rest(0.015);
  }
}
