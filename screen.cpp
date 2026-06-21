#include <allegro5/allegro.h>
#include "bricktron.h"
#include "screen.h"

static ALLEGRO_BITMAP *game_buffer = NULL;

void create_game_buffer() {
  game_buffer = al_create_bitmap(BOARD_WIDTH, BOARD_HEIGHT);
}

void destroy_game_buffer() {
  if (game_buffer) {
    al_destroy_bitmap(game_buffer);
    game_buffer = NULL;
  }
}

void begin_frame() {
  al_set_target_bitmap(game_buffer);
  ALLEGRO_TRANSFORM t;
  al_identity_transform(&t);
  al_use_transform(&t);
  al_clear_to_color(al_map_rgb(0, 0, 0));
}

void end_frame(ALLEGRO_DISPLAY *display) {
  al_set_target_backbuffer(display);
  al_acknowledge_resize(display);

  // Scale the logical buffer to the real backbuffer size. Width and height are
  // scaled independently so the image fills the whole window/fullscreen display
  // (no letterbox bars); on non-4:3 screens this stretches the aspect slightly
  // but keeps the entire board visible.
  ALLEGRO_BITMAP *backbuffer = al_get_backbuffer(display);
  int dw = al_get_bitmap_width(backbuffer);
  int dh = al_get_bitmap_height(backbuffer);

  ALLEGRO_TRANSFORM t;
  al_identity_transform(&t);
  al_use_transform(&t);

  al_clear_to_color(al_map_rgb(0, 0, 0));
  al_draw_scaled_bitmap(game_buffer,
                        0, 0, BOARD_WIDTH, BOARD_HEIGHT,
                        0, 0, (float)dw, (float)dh, 0);
  al_flip_display();
}
