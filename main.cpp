#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cstring>
#include <vector>

#include "arkanoid.h"
#include "screen.h"
#include "game_objects.h"
#include "intro.h"
#include "scores.h"
#include "audio.h"
#include "paths.h"

// All mutable gameplay state, grouped so it can be reset, passed around, and
// later persisted as a unit instead of living in scattered globals.
struct GameState {
  int rozm = 40;
  int x = BOARD_WIDTH / 2 - 40;
  int y = BOARD_HEIGHT - 50;
  bool game_running = false;
  int poziom = 1;
  int score = 0;
  int lives = INITIAL_LIVES;
  char player_name[MAX_NAME_LENGTH + 1] = "";
  int shake_timer = 0;
  // Plays the game-over jingle once per death, re-armed on restart
  bool gameover_sound_done = false;
  // Power-up timers
  int wider_timer = 0;
  int slow_timer = 0;
  int fire_timer = 0;
  int laser_timer = 0;
  int laser_cooldown = 0;
  int catch_timer = 0;
  float paddle_w_mult = PADDLE_WIDTH_MULT;
  bool space_prev = false;
};

std::vector<ball> balls;
tiles game_tiles(BOARD_WIDTH, BOARD_HEIGHT);

// Reset to a single ball resting on the paddle, honoring an active slow timer
void reset_balls(const GameState &gs) {
  balls.clear();
  balls.push_back(ball(BALL_SIZE));
  balls.back().new_game(gs.x, gs.y, gs.rozm);
  if (gs.slow_timer > 0) balls.back().set_speed(BALL_SPEED / 2);
}

// --- Reset game state (keeps player name) ---
void reset_game(GameState &gs) {
  gs.score = 0;
  gs.lives = INITIAL_LIVES;
  gs.poziom = 1;
  gs.game_running = false;
  gs.wider_timer = 0;
  gs.slow_timer = 0;
  gs.fire_timer = 0;
  gs.laser_timer = 0;
  gs.laser_cooldown = 0;
  gs.catch_timer = 0;
  gs.paddle_w_mult = PADDLE_WIDTH_MULT;
  gs.gameover_sound_done = false;
  game_tiles.load_level(gs.poziom);
  reset_balls(gs);
}

// --- Draw HUD ---
void draw_hud(ALLEGRO_FONT *font, const GameState &gs) {
  al_draw_textf(font, al_map_rgb(255, 255, 100), 10, 10, 0,
                "Score: %d", gs.score);
  al_draw_textf(font, al_map_rgb(255, 255, 100), BOARD_WIDTH / 2 - 40, 10, 0,
                "Level %d", gs.poziom);
  al_draw_text(font, al_map_rgba(180, 180, 220, 180), BOARD_WIDTH / 2 + 30,
               10, 0, gs.player_name);
  for (int i = 0; i < gs.lives; i++) {
    float cx = BOARD_WIDTH - 30.0f - i * 25.0f;
    float cy = 16.0f;
    al_draw_filled_circle(cx, cy, 8, al_map_rgba(220, 40, 0, 200));
    al_draw_filled_circle(cx - 1, cy - 2, 4, al_map_rgba(255, 150, 100, 120));
  }

  float ind_y = 30;
  if (gs.wider_timer > 0) {
    float frac = (float)gs.wider_timer / POWERUP_DURATION;
    al_draw_filled_rectangle(10, ind_y, 10 + frac * 80, ind_y + 8,
                             al_map_rgba(40, 200, 40, 180));
    al_draw_text(font, al_map_rgb(40, 220, 40), 95, ind_y - 2, 0, "WIDE");
    ind_y += 14;
  }
  if (gs.slow_timer > 0) {
    float frac = (float)gs.slow_timer / POWERUP_DURATION;
    al_draw_filled_rectangle(10, ind_y, 10 + frac * 80, ind_y + 8,
                             al_map_rgba(40, 100, 240, 180));
    al_draw_text(font, al_map_rgb(40, 100, 255), 95, ind_y - 2, 0, "SLOW");
    ind_y += 14;
  }
  if (gs.fire_timer > 0) {
    float frac = (float)gs.fire_timer / FIRE_DURATION;
    al_draw_filled_rectangle(10, ind_y, 10 + frac * 80, ind_y + 8,
                             al_map_rgba(255, 120, 0, 180));
    al_draw_text(font, al_map_rgb(255, 140, 0), 95, ind_y - 2, 0, "FIRE");
    ind_y += 14;
  }
  if (gs.laser_timer > 0) {
    float frac = (float)gs.laser_timer / LASER_DURATION;
    al_draw_filled_rectangle(10, ind_y, 10 + frac * 80, ind_y + 8,
                             al_map_rgba(0, 200, 255, 180));
    al_draw_text(font, al_map_rgb(0, 220, 255), 95, ind_y - 2, 0, "LASR");
    ind_y += 14;
  }
  if (gs.catch_timer > 0) {
    float frac = (float)gs.catch_timer / CATCH_DURATION;
    al_draw_filled_rectangle(10, ind_y, 10 + frac * 80, ind_y + 8,
                             al_map_rgba(255, 215, 0, 180));
    al_draw_text(font, al_map_rgb(255, 220, 0), 95, ind_y - 2, 0, "CTCH");
  }
}

// --- Draw paddle ---
void draw_paddle(int px, int py, float w_mult, int rozm) {
  float pw = rozm * w_mult;
  float ph = (float)rozm;
  float x1 = (float)px;
  float y1 = (float)py;
  float x2 = x1 + pw;
  float y2 = y1 + ph;

  al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgba(30, 30, 50, 230));
  al_draw_filled_rectangle(x1 + 2, y1, x2 - 2, y1 + 4,
                           al_map_rgba(100, 120, 200, 200));
  al_draw_filled_rectangle(x1 + 4, y1 + 4, x2 - 4, y2 - 4,
                           al_map_rgba(50, 60, 100, 180));
  al_draw_rectangle(x1, y1, x2, y2, al_map_rgba(120, 140, 220, 200), 1.5f);
}

// --- Pause toggle ---
bool paused = false;
bool p_was_down = false;

void toggle_pause(ALLEGRO_KEYBOARD_STATE *kb) {
  bool p_down = al_key_down(kb, ALLEGRO_KEY_P);
  if (p_down && !p_was_down) paused = !paused;
  p_was_down = p_down;
}

// Draw a dimming overlay and centered "PAUSED" banner
void draw_pause_overlay(ALLEGRO_FONT *font) {
  al_draw_filled_rectangle(0, 0, BOARD_WIDTH, BOARD_HEIGHT,
                           al_map_rgba(0, 0, 0, 140));
  const char *title = "PAUSED";
  const char *hint = "Press P to resume";
  float cx = BOARD_WIDTH / 2.0f;
  float cy = BOARD_HEIGHT / 2.0f;
  al_draw_text(font, al_map_rgb(255, 255, 100), cx, cy - 20,
               ALLEGRO_ALIGN_CENTER, title);
  al_draw_text(font, al_map_rgba(200, 200, 220, 200), cx, cy + 4,
               ALLEGRO_ALIGN_CENTER, hint);
}

// --- Fullscreen toggle ---
bool fullscreen = false;
bool f_was_down = false;

void toggle_fullscreen(ALLEGRO_DISPLAY *display, ALLEGRO_KEYBOARD_STATE *kb) {
  bool f_down = al_key_down(kb, ALLEGRO_KEY_F);
  if (f_down && !f_was_down) {
    fullscreen = !fullscreen;
    al_set_display_flag(display, ALLEGRO_FULLSCREEN_WINDOW, fullscreen);
  }
  f_was_down = f_down;
}

// True if any keyboard key is currently held (used to leave attract mode).
static bool any_key_down(ALLEGRO_KEYBOARD_STATE *kb) {
  for (int k = 1; k < ALLEGRO_KEY_MAX; k++)
    if (al_key_down(kb, k)) return true;
  return false;
}

// ============================================================
//  Main
// ============================================================

int main(int argc, char **argv) {
  // --demo: hands-free attract mode (auto-launch, paddle tracks the ball,
  // auto-restart on game over) used for recording gameplay footage.
  bool demo_mode = false;
  for (int i = 1; i < argc; i++)
    if (strcmp(argv[i], "--demo") == 0) demo_mode = true;

  al_init();
  srand((unsigned int)time(NULL));
  al_install_keyboard();
  al_init_font_addon();
  al_init_image_addon();
  al_init_primitives_addon();
  init_audio();

  ALLEGRO_KEYBOARD_STATE klawiatura;
  al_set_new_display_flags(ALLEGRO_RESIZABLE);
  ALLEGRO_DISPLAY *okno = al_create_display(BOARD_WIDTH, BOARD_HEIGHT);
  al_set_window_title(okno, "Arkanoid by Artur Kos");

  create_game_buffer();

  ALLEGRO_BITMAP *background = NULL;
  ALLEGRO_FONT *font8 = al_create_builtin_font();

  al_set_target_bitmap(al_get_backbuffer(okno));

  background = al_load_bitmap(data_path(BACKGROUND_FILE).c_str());
  if (!background) {
    fprintf(stderr,
            "Error: Failed to load '%s'. "
            "Make sure the file exists in the working directory.\n",
            BACKGROUND_FILE);
    destroy_audio();
    destroy_game_buffer();
    al_destroy_display(okno);
    return -1;
  }

  play_sound(SND_START);

  // The whole program is one outer loop: intro screen -> a single playthrough
  // -> back to intro. The attract/demo path (inactivity on the intro, or the
  // --demo flag) drives the paddle itself.
  bool program_quit = false;
  while (!program_quit) {
    GameState gs;
    bool attract = false;

    // --- Retro intro screen ---
    if (demo_mode) {
      strncpy(gs.player_name, "DEMO", MAX_NAME_LENGTH);
      attract = true;
    } else {
      intro_result ir = run_intro(okno, font8);
      if (ir == INTRO_QUIT) {
        program_quit = true;
        break;
      } else if (ir == INTRO_ATTRACT) {
        attract = true;
        strncpy(gs.player_name, "DEMO", MAX_NAME_LENGTH);
      } else {
        // --- Name prompt ---
        if (!prompt_name(okno, font8, gs.player_name))
          continue;  // cancelled -> back to the intro

        // Wait for key release
        do {
          al_get_keyboard_state(&klawiatura);
          al_rest(0.01);
        } while (al_key_down(&klawiatura, ALLEGRO_KEY_ENTER) ||
                 al_key_down(&klawiatura, ALLEGRO_KEY_SPACE));
      }
    }

    // The paddle plays itself in either attract or --demo mode. Attract started
    // from the intro returns there on any key; --demo only exits on ESC.
    bool ai_paddle = demo_mode || attract;
    bool attract_to_intro = attract && !demo_mode;

    reset_game(gs);
    paused = false;
    double czas = al_get_time();

  while (true) {
    al_get_keyboard_state(&klawiatura);

    // --- Exit conditions ---
    if (attract_to_intro) {
      if (any_key_down(&klawiatura)) break;  // back to the intro
    } else if (al_key_down(&klawiatura, ALLEGRO_KEY_ESCAPE)) {
      program_quit = true;
      break;
    }

    toggle_fullscreen(okno, &klawiatura);
    if (!ai_paddle) toggle_pause(&klawiatura);

    // --- Game over: show high scores ---
    if (gs.lives <= 0) {
      // Demo mode never stops to ask anything; just start a fresh game
      if (ai_paddle) {
        reset_game(gs);
        continue;
      }
      if (!gs.gameover_sound_done) {
        play_sound(SND_GAMEOVER);
        gs.gameover_sound_done = true;
      }
      bool restart =
          draw_high_scores(okno, font8, gs.player_name, gs.score, gs.poziom);
      if (restart) {
        reset_game(gs);
        do {
          al_get_keyboard_state(&klawiatura);
          al_rest(0.01);
        } while (al_key_down(&klawiatura, ALLEGRO_KEY_SPACE));
      } else {
        program_quit = true;
        break;
      }
      continue;
    }

    // --- Paused: draw a frozen frame with overlay, skip all updates ---
    if (paused) {
      begin_frame();
      ALLEGRO_TRANSFORM tp;
      al_identity_transform(&tp);
      al_use_transform(&tp);
      al_draw_scaled_bitmap(background, 0, 0,
                            (float)al_get_bitmap_width(background),
                            (float)al_get_bitmap_height(background),
                            0, 0, BOARD_WIDTH, BOARD_HEIGHT, 0);
      draw_paddle(gs.x, gs.y, gs.paddle_w_mult, gs.rozm);
      game_tiles.draw_tiles();
      game_tiles.draw_powerups();
      for (auto &b : balls) {
        b.draw_trail();
        b.draw_ball();
      }
      draw_hud(font8, gs);
      draw_pause_overlay(font8);
      end_frame(okno);
      al_rest(0.01);
      continue;
    }

    // --- Input ---
    if (al_get_time() > czas + 0.001) {
      float pw = gs.rozm * gs.paddle_w_mult;
      if (ai_paddle) {
        // Auto-launch, then steer the paddle. Normally chase the lowest ball
        // (the one most likely to fall); but if a power-up is dropping and the
        // ball is not an immediate threat, go grab the power-up instead.
        gs.game_running = true;
        for (auto &b : balls) if (b.is_stuck()) b.release();
        ball *target = nullptr;
        for (auto &b : balls)
          if (!target || b.get_y() > target->get_y()) target = &b;
        float aim = target ? (float)target->get_x() : -1.0f;
        float pcx, py;
        bool have_pu = game_tiles.lowest_powerup(pcx, py);
        (void)py;
        bool ball_safe = !target || target->get_y() < BOARD_HEIGHT / 2 ||
                         target->get_ry_move() < 0;
        if (have_pu && ball_safe) aim = pcx;
        if (aim >= 0.0f) {
          int desired = (int)(aim - pw / 2);
          if (gs.x < desired && gs.x < BOARD_WIDTH - (int)pw)
            gs.x += PADDLE_SPEED;
          else if (gs.x > desired && gs.x > 0)
            gs.x -= PADDLE_SPEED;
        }
      } else {
        if (al_key_down(&klawiatura, ALLEGRO_KEY_RIGHT) &&
            gs.x < BOARD_WIDTH - (int)pw)
          gs.x += PADDLE_SPEED;
        if (al_key_down(&klawiatura, ALLEGRO_KEY_LEFT) && gs.x > 0)
          gs.x -= PADDLE_SPEED;
        if (al_key_down(&klawiatura, ALLEGRO_KEY_DOWN) &&
            gs.y < BOARD_HEIGHT - gs.rozm - 10)
          gs.y += PADDLE_SPEED;
        if (al_key_down(&klawiatura, ALLEGRO_KEY_UP) && gs.y > BOARD_HEIGHT / 2)
          gs.y -= PADDLE_SPEED;
        if (al_key_down(&klawiatura, ALLEGRO_KEY_SPACE) && !gs.game_running)
          gs.game_running = true;
        // Edge-detect SPACE: release any stuck balls first, else fire laser
        {
          bool space_now = al_key_down(&klawiatura, ALLEGRO_KEY_SPACE);
          bool space_edge = space_now && !gs.space_prev;
          gs.space_prev = space_now;
          bool any_stuck = false;
          for (auto &b : balls) if (b.is_stuck()) { any_stuck = true; break; }
          if (space_edge && gs.game_running && any_stuck) {
            for (auto &b : balls) if (b.is_stuck()) b.release();
          } else if (space_now && gs.game_running && gs.laser_timer > 0 &&
                     gs.laser_cooldown == 0) {
            game_tiles.fire_lasers((float)gs.x, pw, (float)gs.y);
            gs.laser_cooldown = LASER_COOLDOWN;
            play_sound(SND_PADDLE);
          }
        }
      }
      czas = al_get_time();
    }

    // --- Power-up timers ---
    if (gs.wider_timer > 0) {
      gs.wider_timer--;
      gs.paddle_w_mult = PADDLE_WIDER_MULT;
      if (gs.wider_timer == 0) gs.paddle_w_mult = PADDLE_WIDTH_MULT;
    }
    if (gs.slow_timer > 0) {
      gs.slow_timer--;
      if (gs.slow_timer == 0)
        for (auto &b : balls) b.set_speed(BALL_SPEED);
    }
    if (gs.fire_timer > 0) gs.fire_timer--;
    if (gs.laser_timer > 0) gs.laser_timer--;
    if (gs.laser_cooldown > 0) gs.laser_cooldown--;
    if (gs.catch_timer > 0) {
      gs.catch_timer--;
      if (gs.catch_timer == 0)
        for (auto &b : balls) if (b.is_stuck()) b.release();
    }

    // --- Draw to buffer ---
    begin_frame();

    // Screen shake offset
    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    if (gs.shake_timer > 0) {
      float intensity = (float)gs.shake_timer / SHAKE_FRAMES * SHAKE_INTENSITY;
      float shx = (float)(rand() % (int)(intensity * 2 + 1)) - intensity;
      float shy = (float)(rand() % (int)(intensity * 2 + 1)) - intensity;
      al_translate_transform(&t, shx, shy);
      gs.shake_timer--;
    }
    al_use_transform(&t);

    // Background
    al_draw_scaled_bitmap(background, 0, 0,
                          (float)al_get_bitmap_width(background),
                          (float)al_get_bitmap_height(background),
                          0, 0, BOARD_WIDTH, BOARD_HEIGHT, 0);

    // Paddle
    draw_paddle(gs.x, gs.y, gs.paddle_w_mult, gs.rozm);

    // Tiles + collisions (each ball, fireball plows through when active)
    game_tiles.draw_tiles();
    bool fire_on = gs.fire_timer > 0;
    for (auto &b : balls) {
      b.set_fire(fire_on);
      gs.score += game_tiles.check_collisions(&b, gs.game_running,
                                              &gs.shake_timer, fire_on);
    }

    // Particles
    game_tiles.update_and_draw_particles();

    // Power-ups
    game_tiles.update_powerups();
    game_tiles.draw_powerups();
    int pu = game_tiles.collect_powerup((float)gs.x, (float)gs.y,
                                        gs.rozm * gs.paddle_w_mult,
                                        (float)gs.rozm);
    if (pu != 0) play_sound(SND_POWERUP);
    if (pu == POWERUP_WIDER) {
      gs.wider_timer = POWERUP_DURATION;
      gs.paddle_w_mult = PADDLE_WIDER_MULT;
    } else if (pu == POWERUP_SLOW) {
      gs.slow_timer = POWERUP_DURATION;
      for (auto &b : balls) b.set_speed(BALL_SPEED / 2);
    } else if (pu == POWERUP_LIFE) {
      gs.lives++;
    } else if (pu == POWERUP_MULTI) {
      // Spawn extra balls diverging upward from the first ball, capped total
      int spawn = MULTIBALL_ADD;
      int speed = (gs.slow_timer > 0) ? BALL_SPEED / 2 : BALL_SPEED;
      for (int i = 0; i < spawn && (int)balls.size() < 6; i++) {
        ball nb = balls[0];
        nb.set_velocity((i % 2 == 0) ? speed : -speed, -speed);
        balls.push_back(nb);
      }
      gs.game_running = true;
    } else if (pu == POWERUP_FIRE) {
      gs.fire_timer = FIRE_DURATION;
    } else if (pu == POWERUP_LASER) {
      gs.laser_timer = LASER_DURATION;
    } else if (pu == POWERUP_CATCH) {
      gs.catch_timer = CATCH_DURATION;
    }

    // Laser bolts
    gs.score += game_tiles.update_lasers(&gs.shake_timer);
    game_tiles.draw_lasers();

    // Ball movement; drop balls that fell off the bottom
    for (int i = (int)balls.size() - 1; i >= 0; i--) {
      if (balls[i].make_ball_move(gs.x, gs.y, gs.rozm, gs.paddle_w_mult,
                                  gs.game_running, gs.catch_timer > 0)) {
        balls.erase(balls.begin() + i);
      }
    }
    // Losing the last ball costs a life and re-arms a ball on the paddle
    if (gs.game_running && balls.empty()) {
      gs.lives--;
      play_sound(SND_LIFE_LOST);
      gs.game_running = false;
      gs.fire_timer = 0;
      gs.catch_timer = 0;
      reset_balls(gs);
    }

    // Level complete
    if (game_tiles.game_over()) {
      play_sound(SND_LEVEL);
      gs.game_running = false;
      gs.poziom++;
      gs.wider_timer = 0;
      gs.slow_timer = 0;
      gs.fire_timer = 0;
      gs.laser_timer = 0;
      gs.laser_cooldown = 0;
      gs.catch_timer = 0;
      gs.paddle_w_mult = PADDLE_WIDTH_MULT;
      game_tiles.load_level(gs.poziom);
      reset_balls(gs);
    }

    // HUD (no shake)
    al_identity_transform(&t);
    al_use_transform(&t);
    draw_hud(font8, gs);
    if (ai_paddle)
      al_draw_text(font8, al_map_rgba(255, 255, 0, 200), BOARD_WIDTH / 2.0f,
                   BOARD_HEIGHT - 20.0f, ALLEGRO_ALIGN_CENTER, "DEMO");

    // --- Stretch buffer to display ---
    end_frame(okno);
    al_rest(0.01);
  }  // inner gameplay loop
  }  // outer program loop -> back to the intro unless program_quit

  al_destroy_bitmap(background);
  al_destroy_font(font8);
  destroy_audio();
  destroy_game_buffer();
  al_destroy_display(okno);
  return 0;
}
