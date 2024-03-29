#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>

#include <vector>

#include "arkanoid.h"
#include "game_objects.cpp"

int rozm = 40;
bool game_running;
int x = BOARD_WIDTH / 2 - rozm, y = BOARD_HEIGHT - rozm,
    poziom = 1;  //, rx = 50, ry = 50, rx_move = 6, ry_move = 6, poziom = 1;

ball game_ball(BALL_SIZE);
tiles game_tiles(BOARD_WIDTH, BOARD_HEIGHT, &game_ball);

int main() {
  al_init();
  al_install_keyboard();
  al_init_font_addon();
  al_init_image_addon();
  ALLEGRO_KEYBOARD_STATE klawiatura;

  ALLEGRO_DISPLAY *okno = al_create_display(BOARD_WIDTH, BOARD_HEIGHT);
  game_running = false;
  al_set_window_title(okno, "Arkanoid by Artur Kos");

  ALLEGRO_BITMAP *background = NULL;
  ALLEGRO_FONT *font8 = al_create_builtin_font();

  al_clear_to_color(al_map_rgb(200, 0, 0));
  al_set_target_bitmap(al_get_backbuffer(okno));

  double czas = al_get_time();
  background = al_load_bitmap(BACKGROUND_FILE);
  if (!background) {
    return -1;
  }
  game_ball.set_draw_function(al_draw_filled_circle);
  game_tiles.set_draw_function(al_draw_filled_rectangle);
  while (!al_key_down(&klawiatura, ALLEGRO_KEY_ESCAPE)) {
    al_get_keyboard_state(&klawiatura);
    if (al_get_time() > czas + 0.001) {
      if (al_key_down(&klawiatura, ALLEGRO_KEY_RIGHT) &&
          x < BOARD_WIDTH - rozm * 3)
        x = x + 10;
      if (al_key_down(&klawiatura, ALLEGRO_KEY_LEFT) && x > 0) x = x - 10;
      if (al_key_down(&klawiatura, ALLEGRO_KEY_DOWN) && y < BOARD_HEIGHT - rozm)
        y = y + 10;
      if (al_key_down(&klawiatura, ALLEGRO_KEY_UP) && y > BOARD_HEIGHT / 2)
        y = y - 10;
      if (al_key_down(&klawiatura, ALLEGRO_KEY_SPACE) && game_running == false)
        game_running = true;
      czas = al_get_time();
    }
    al_draw_scaled_bitmap(background, 0, 0, al_get_bitmap_width(background),
                          al_get_bitmap_height(background), 0, 0, BOARD_WIDTH,
                          BOARD_HEIGHT, 0);
    al_draw_filled_rectangle(x, y, x + rozm * 3, y + rozm,
                             al_map_rgba(0, 0, 0, 100));
    game_tiles.check_collisions(game_running);
    game_tiles.play_animation_when_tile_and_ball_collision();
    game_ball.make_ball_move(x, y, rozm, &game_running);
    if (game_tiles.game_over()) {
      game_running = false;
      poziom++;
      game_tiles.new_game();
      game_ball.new_game(x, y, rozm);
    }
    al_draw_textf(font8, al_map_rgb(255, 255, 0), 10, 10, 0, "Poziom %3d",
                  poziom);
    al_flip_display();
    al_rest(0.01);
  }
  al_destroy_bitmap(background);
  al_destroy_display(okno);
  return 0;
}
