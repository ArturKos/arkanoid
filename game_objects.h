#ifndef GAME_OBJECTS_H
#define GAME_OBJECTS_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <vector>

#include "arkanoid.h"

// --- Particle (improved explosion) ---
struct particle {
  float x, y;
  float vx, vy;
  float alpha;
  unsigned char r, g, b;
};

// --- Ball trail point ---
struct trail_point {
  float x, y;
};

// --- Power-up falling from destroyed tile ---
struct powerup {
  float x, y;
  int type;
  bool active;
};

// --- Ball ---
class ball {
 private:
  int rx, ry;
  int rx_move, ry_move;
  int speed;
  trail_point trail[BALL_TRAIL_LENGTH];
  int trail_count;

 public:
  ball(int size);
  void new_game(int x, int y, int rozm);
  int get_x();
  int get_y();
  int get_ry_move();
  void make_ball_move(int x, int y, int rozm, float paddle_w_mult,
                      bool *game_running, int *lives);
  void reverse_y();
  void reverse_x();
  void draw_ball();
  void draw_trail();
  void set_speed(int s);
  int get_speed();
};

// --- Single tile ---
class tile {
 private:
  int tile_x, tile_y, tile_width, tile_height;
  unsigned char base_r, base_g, base_b;
  bool tile_visible;
  int hp, max_hp;

 public:
  tile(int x, int y, int w, int h, unsigned char r, unsigned char g,
       unsigned char b, int hp);
  int get_x();
  int get_y();
  int get_width();
  int get_height();
  int get_hp();
  int get_max_hp();
  bool get_visible();
  void set_visible(bool vis);
  void set_hp(int h);
  void set_color(unsigned char r, unsigned char g, unsigned char b);
  void get_color(unsigned char &r, unsigned char &g, unsigned char &b);
  bool hit();  // returns true if tile is now destroyed
  void draw_beveled();
};

// --- Tile grid + particles + powerups ---
class tiles {
 private:
  tile *game_tiles[TILES_IN_COLUMN * TILES_IN_ROW];
  std::vector<particle> particles;
  std::vector<powerup> powerups;
  ball *game_ball;
  int size_width;
  int size_height;

 public:
  tiles(int board_width, int board_height, ball *gball);
  ~tiles();
  int check_collisions(bool game_running, int *shake);
  bool game_over();
  void new_game();
  void draw_tiles();
  void spawn_particles(int x, int y, int w, int h, unsigned char r,
                       unsigned char g, unsigned char b);
  void update_and_draw_particles();
  void update_powerups();
  void draw_powerups(ALLEGRO_FONT *font);
  int collect_powerup(float paddle_x, float paddle_y, float paddle_w,
                      float paddle_h);
};

#endif
