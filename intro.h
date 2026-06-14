#ifndef INTRO_H
#define INTRO_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

// Intro outcome.
enum intro_result { INTRO_QUIT = 0, INTRO_PLAY = 1, INTRO_ATTRACT = 2 };

// Runs the intro screen. Returns INTRO_PLAY on SPACE, INTRO_QUIT on ESC, or
// INTRO_ATTRACT after INTRO_ATTRACT_SECONDS of inactivity (start the demo).
intro_result run_intro(ALLEGRO_DISPLAY *display, ALLEGRO_FONT *font);

#endif
