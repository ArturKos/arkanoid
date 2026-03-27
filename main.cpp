#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cstring>

#include "arkanoid.h"
#include "screen.h"
#include "game_objects.h"
#include "intro.h"
#include "scores.h"

// --- Game state ---
int rozm = 40;
bool game_running;
int x = BOARD_WIDTH / 2 - 40, y = BOARD_HEIGHT - 50;
int poziom = 1;
int score = 0;
int lives = INITIAL_LIVES;
char player_name[MAX_NAME_LENGTH + 1] = "";

// Screen shake
int shake_timer = 0;

// Power-up timers
int wider_timer = 0;
int slow_timer = 0;
float paddle_w_mult = PADDLE_WIDTH_MULT;

ball game_ball(BALL_SIZE);
tiles game_tiles(BOARD_WIDTH, BOARD_HEIGHT, &game_ball);

// --- Reset game state ---
void reset_game() {
  score = 0;
  lives = INITIAL_LIVES;
  poziom = 1;
  game_running = false;
  wider_timer = 0;
  slow_timer = 0;
  paddle_w_mult = PADDLE_WIDTH_MULT;
  game_ball.set_speed(BALL_SPEED);
  game_tiles.new_game();
  game_ball.new_game(x, y, rozm);
}

// --- Draw HUD ---
void draw_hud(ALLEGRO_FONT *font) {
  al_draw_textf(font, al_map_rgb(255, 255, 100), 10, 10, 0,
                "Score: %d", score);
  al_draw_textf(font, al_map_rgb(255, 255, 100), BOARD_WIDTH / 2 - 40, 10, 0,
                "Level %d", poziom);
  al_draw_text(font, al_map_rgba(180, 180, 220, 180), BOARD_WIDTH / 2 + 30,
               10, 0, player_name);
  for (int i = 0; i < lives; i++) {
    float cx = BOARD_WIDTH - 30.0f - i * 25.0f;
    float cy = 16.0f;
    al_draw_filled_circle(cx, cy, 8, al_map_rgba(220, 40, 0, 200));
    al_draw_filled_circle(cx - 1, cy - 2, 4, al_map_rgba(255, 150, 100, 120));
  }

  float ind_y = 30;
  if (wider_timer > 0) {
    float frac = (float)wider_timer / POWERUP_DURATION;
    al_draw_filled_rectangle(10, ind_y, 10 + frac * 80, ind_y + 8,
                             al_map_rgba(40, 200, 40, 180));
    al_draw_text(font, al_map_rgb(40, 220, 40), 95, ind_y - 2, 0, "WIDE");
    ind_y += 14;
  }
  if (slow_timer > 0) {
    float frac = (float)slow_timer / POWERUP_DURATION;
    al_draw_filled_rectangle(10, ind_y, 10 + frac * 80, ind_y + 8,
                             al_map_rgba(40, 100, 240, 180));
    al_draw_text(font, al_map_rgb(40, 100, 255), 95, ind_y - 2, 0, "SLOW");
  }
}

// --- Draw paddle ---
void draw_paddle(int px, int py, float w_mult) {
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

// ============================================================
//  Main
// ============================================================

int main() {
  al_init();
  srand((unsigned int)time(NULL));
  al_install_keyboard();
  al_init_font_addon();
  al_init_image_addon();
  al_init_primitives_addon();

  ALLEGRO_KEYBOARD_STATE klawiatura;
  al_set_new_display_flags(ALLEGRO_RESIZABLE);
  ALLEGRO_DISPLAY *okno = al_create_display(BOARD_WIDTH, BOARD_HEIGHT);
  game_running = false;
  al_set_window_title(okno, "Arkanoid by Artur Kos");

  create_game_buffer();

  ALLEGRO_BITMAP *background = NULL;
  ALLEGRO_FONT *font8 = al_create_builtin_font();

  al_set_target_bitmap(al_get_backbuffer(okno));

  background = al_load_bitmap(BACKGROUND_FILE);
  if (!background) {
    fprintf(stderr,
            "Error: Failed to load '%s'. "
            "Make sure the file exists in the working directory.\n",
            BACKGROUND_FILE);
    destroy_game_buffer();
    al_destroy_display(okno);
    return -1;
  }

  // --- Retro intro screen ---
  if (!run_intro(okno, font8)) {
    al_destroy_bitmap(background);
    al_destroy_font(font8);
    destroy_game_buffer();
    al_destroy_display(okno);
    return 0;
  }

  // --- Name prompt ---
  if (!prompt_name(okno, font8, player_name)) {
    al_destroy_bitmap(background);
    al_destroy_font(font8);
    destroy_game_buffer();
    al_destroy_display(okno);
    return 0;
  }

  // Wait for key release
  do {
    al_get_keyboard_state(&klawiatura);
    al_rest(0.01);
  } while (al_key_down(&klawiatura, ALLEGRO_KEY_ENTER) ||
           al_key_down(&klawiatura, ALLEGRO_KEY_SPACE));

  // Reset ball to sit on the paddle
  game_ball.new_game(x, y, rozm);
  double czas = al_get_time();

  while (!al_key_down(&klawiatura, ALLEGRO_KEY_ESCAPE)) {
    al_get_keyboard_state(&klawiatura);
    toggle_fullscreen(okno, &klawiatura);

    // --- Game over: show high scores ---
    if (lives <= 0) {
      bool restart = draw_high_scores(okno, font8, player_name, score, poziom);
      if (restart) {
        reset_game();
        do {
          al_get_keyboard_state(&klawiatura);
          al_rest(0.01);
        } while (al_key_down(&klawiatura, ALLEGRO_KEY_SPACE));
      } else {
        break;
      }
      continue;
    }

    // --- Input ---
    if (al_get_time() > czas + 0.001) {
      float pw = rozm * paddle_w_mult;
      if (al_key_down(&klawiatura, ALLEGRO_KEY_RIGHT) &&
          x < BOARD_WIDTH - (int)pw)
        x += PADDLE_SPEED;
      if (al_key_down(&klawiatura, ALLEGRO_KEY_LEFT) && x > 0)
        x -= PADDLE_SPEED;
      if (al_key_down(&klawiatura, ALLEGRO_KEY_DOWN) &&
          y < BOARD_HEIGHT - rozm - 10)
        y += PADDLE_SPEED;
      if (al_key_down(&klawiatura, ALLEGRO_KEY_UP) && y > BOARD_HEIGHT / 2)
        y -= PADDLE_SPEED;
      if (al_key_down(&klawiatura, ALLEGRO_KEY_SPACE) && !game_running)
        game_running = true;
      czas = al_get_time();
    }

    // --- Power-up timers ---
    if (wider_timer > 0) {
      wider_timer--;
      paddle_w_mult = PADDLE_WIDER_MULT;
      if (wider_timer == 0) paddle_w_mult = PADDLE_WIDTH_MULT;
    }
    if (slow_timer > 0) {
      slow_timer--;
      if (slow_timer == 0) game_ball.set_speed(BALL_SPEED);
    }

    // --- Draw to buffer ---
    begin_frame();

    // Screen shake offset
    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    if (shake_timer > 0) {
      float intensity = (float)shake_timer / SHAKE_FRAMES * SHAKE_INTENSITY;
      float shx = (float)(rand() % (int)(intensity * 2 + 1)) - intensity;
      float shy = (float)(rand() % (int)(intensity * 2 + 1)) - intensity;
      al_translate_transform(&t, shx, shy);
      shake_timer--;
    }
    al_use_transform(&t);

    // Background
    al_draw_scaled_bitmap(background, 0, 0,
                          (float)al_get_bitmap_width(background),
                          (float)al_get_bitmap_height(background),
                          0, 0, BOARD_WIDTH, BOARD_HEIGHT, 0);

    // Paddle
    draw_paddle(x, y, paddle_w_mult);

    // Tiles + collisions
    game_tiles.draw_tiles();
    int gained = game_tiles.check_collisions(game_running, &shake_timer);
    score += gained;

    // Particles
    game_tiles.update_and_draw_particles();

    // Power-ups
    game_tiles.update_powerups();
    game_tiles.draw_powerups(font8);
    int pu = game_tiles.collect_powerup((float)x, (float)y,
                                        rozm * paddle_w_mult, (float)rozm);
    if (pu == POWERUP_WIDER) {
      wider_timer = POWERUP_DURATION;
      paddle_w_mult = PADDLE_WIDER_MULT;
    } else if (pu == POWERUP_SLOW) {
      slow_timer = POWERUP_DURATION;
      game_ball.set_speed(BALL_SPEED / 2);
    } else if (pu == POWERUP_LIFE) {
      lives++;
    }

    // Ball
    game_ball.make_ball_move(x, y, rozm, paddle_w_mult,
                            &game_running, &lives);

    // Level complete
    if (game_tiles.game_over()) {
      game_running = false;
      poziom++;
      wider_timer = 0;
      slow_timer = 0;
      paddle_w_mult = PADDLE_WIDTH_MULT;
      game_ball.set_speed(BALL_SPEED);
      game_tiles.new_game();
      game_ball.new_game(x, y, rozm);
    }

    // HUD (no shake)
    al_identity_transform(&t);
    al_use_transform(&t);
    draw_hud(font8);

    // --- Stretch buffer to display ---
    end_frame(okno);
    al_rest(0.01);
  }

  al_destroy_bitmap(background);
  al_destroy_font(font8);
  destroy_game_buffer();
  al_destroy_display(okno);
  return 0;
}
