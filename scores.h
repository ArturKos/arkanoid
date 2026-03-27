#ifndef SCORES_H
#define SCORES_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <string>
#include <vector>

#define SCORES_FILE "scores.dat"
#define MAX_HIGH_SCORES 10
#define MAX_NAME_LENGTH 16

struct score_entry {
  char name[MAX_NAME_LENGTH + 1];
  int score;
  int level;
};

// Load high scores from file (sorted descending)
std::vector<score_entry> load_scores();

// Save a new entry, keeps top MAX_HIGH_SCORES
void save_score(const char *name, int score, int level);

// Name input screen — returns true if name entered, false if ESC
bool prompt_name(ALLEGRO_DISPLAY *display, ALLEGRO_FONT *font, char *out_name);

// Draw high score table overlay, returns true when user presses SPACE
bool draw_high_scores(ALLEGRO_DISPLAY *display, ALLEGRO_FONT *font,
                      const char *player_name, int player_score,
                      int player_level);

#endif
