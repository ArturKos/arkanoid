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

/**
 * @brief A single laser bolt fired by the paddle during POWERUP_LASER.
 *
 * Two bolts are spawned per SPACE press (one from each paddle edge).
 * A bolt is erased on first tile contact or when it exits the top of the
 * board; @c active flags it as in-flight.
 */
struct laser {
  float x;     ///< Left edge of the bolt in logical screen coordinates.
  float y;     ///< Top edge of the bolt in logical screen coordinates.
  bool active; ///< True while the bolt is still travelling upward.
};

// --- Ball ---
class ball {
 private:
  int rx, ry;
  int rx_move, ry_move;
  int speed;
  trail_point trail[BALL_TRAIL_LENGTH];
  int trail_count;
  bool on_fire;
  bool stuck;          ///< True while the ball is glued to the paddle (CATCH).
  float stuck_offset;  ///< Ball centre x relative to paddle left edge.

 public:
  ball(int size);
  void new_game(int x, int y, int rozm);
  int get_x();
  int get_y();
  int get_ry_move();
  /** @brief Returns true while the ball is caught/stuck to the paddle. */
  bool is_stuck() const;
  /** @brief Releases the ball from the paddle, ensuring upward travel. */
  void release();
  // Advances and draws the ball; returns 1 if it fell off the bottom this
  // frame (the caller decides what that means for lives / ball count).
  // @param catch_active  True while the CATCH power-up timer is running.
  int make_ball_move(int x, int y, int rozm, float paddle_w_mult,
                     bool game_running, bool catch_active);
  void reverse_y();
  void reverse_x();
  void draw_ball();
  void draw_trail();
  void set_speed(int s);
  int get_speed();
  void set_fire(bool f);
  void set_velocity(int mx, int my);
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
  std::vector<laser> lasers;
  int size_width;
  int size_height;

 public:
  tiles(int board_width, int board_height);
  ~tiles();
  /**
   * @brief Ball-vs-tile collision detection and resolution.
   * @param b            The ball to test.
   * @param game_running True while the game is actively running.
   * @param shake        Screen-shake frame counter; set to SHAKE_FRAMES on destroy.
   * @param flash        Hit-flash frame counter; set to FLASH_FRAMES on destroy.
   *                     May be nullptr (e.g. demo/attract mode).
   * @param fireball     True while the FIRE power-up is active.
   * @return Score gained from hits and destroys this frame.
   */
  int check_collisions(ball *b, bool game_running, int *shake, int *flash,
                       bool fireball);
  bool game_over();
  void new_game();
  // Load layout from levels/NN.txt; falls back to a random grid if absent.
  bool load_level(int level);
  void draw_tiles();
  void spawn_particles(int x, int y, int w, int h, unsigned char r,
                       unsigned char g, unsigned char b);
  void update_and_draw_particles();
  void update_powerups();
  void draw_powerups();
  int collect_powerup(float paddle_x, float paddle_y, float paddle_w,
                      float paddle_h);
  // For demo mode: the lowest (closest-to-paddle) active power-up. Returns
  // false if none are falling; otherwise fills its centre x and top y.
  bool lowest_powerup(float &out_cx, float &out_y);

  /** @brief Fire two bolts from near the left and right edges of the paddle. */
  void fire_lasers(float paddle_x, float paddle_w, float paddle_y);

  /**
   * @brief Move all active laser bolts upward and resolve tile collisions.
   * @param shake Pointer to the screen-shake frame counter; set to SHAKE_FRAMES
   *              on each tile destroy.
   * @param flash Pointer to the hit-flash frame counter; set to FLASH_FRAMES on
   *              each tile destroy.  May be nullptr.
   * @return Score gained from tile hits/destroys this frame.
   */
  int update_lasers(int *shake, int *flash);

  /** @brief Draw all active laser bolts onto the logical frame buffer. */
  void draw_lasers();
};

#endif
