#include <allegro5/allegro.h>
#include "arkanoid.h"
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

  // Always get monitor info for the most reliable dimensions
  ALLEGRO_MONITOR_INFO info;
  al_get_monitor_info(0, &info);
  int monitor_w = info.x2 - info.x1;
  int monitor_h = info.y2 - info.y1;

  int dw, dh;
  int flags = al_get_display_flags(display);
  if (flags & ALLEGRO_FULLSCREEN_WINDOW) {
    dw = monitor_w;
    dh = monitor_h;
  } else {
    dw = al_get_display_width(display);
    dh = al_get_display_height(display);
  }

  float sx = (float)dw / BOARD_WIDTH;
  float sy = (float)dh / BOARD_HEIGHT;
  float scale = (sx < sy) ? sx : sy;
  float ox = (dw - BOARD_WIDTH * scale) / 2.0f;
  float oy = (dh - BOARD_HEIGHT * scale) / 2.0f;

  // Use a transform to position and scale the buffer blit
  ALLEGRO_TRANSFORM t;
  al_identity_transform(&t);
  al_use_transform(&t);

  al_clear_to_color(al_map_rgb(0, 0, 0));
  al_draw_scaled_bitmap(game_buffer,
                        0, 0, BOARD_WIDTH, BOARD_HEIGHT,
                        ox, oy, BOARD_WIDTH * scale, BOARD_HEIGHT * scale, 0);
  al_flip_display();
}
