#ifndef SCREEN_H
#define SCREEN_H

#include <allegro5/allegro.h>
#include "arkanoid.h"

void create_game_buffer();
void destroy_game_buffer();
void begin_frame();
void end_frame(ALLEGRO_DISPLAY *display);

#endif
