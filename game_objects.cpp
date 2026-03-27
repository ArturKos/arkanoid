#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <cstdlib>
#include <cmath>

#include "arkanoid.h"
#include "game_objects.h"

// ============================================================
//  Ball
// ============================================================

ball::ball(int size) {
  (void)size;
  speed = BALL_SPEED;
  trail_count = 0;
  new_game(0, 0, 0);
}

void ball::new_game(int x, int y, int rozm) {
  rx = x + (rozm * PADDLE_WIDTH_MULT) / 2;
  ry = y - BALL_SIZE;
  speed = BALL_SPEED;
  ry_move = -speed;
  rx_move = (rand() % 2 == 0) ? speed : -speed;
  trail_count = 0;
}

int ball::get_x() { return rx; }
int ball::get_y() { return ry; }
int ball::get_ry_move() { return ry_move; }
void ball::reverse_y() { ry_move = -ry_move; }
void ball::reverse_x() { rx_move = -rx_move; }

void ball::set_speed(int s) {
  rx_move = (rx_move > 0) ? s : -s;
  ry_move = (ry_move > 0) ? s : -s;
  speed = s;
}

int ball::get_speed() { return speed; }

void ball::make_ball_move(int x, int y, int rozm, float paddle_w_mult,
                          bool *game_running, int *lives) {
  if (*game_running) {
    // Store trail point
    if (trail_count < BALL_TRAIL_LENGTH) {
      trail[trail_count].x = (float)rx;
      trail[trail_count].y = (float)ry;
      trail_count++;
    } else {
      for (int i = 0; i < BALL_TRAIL_LENGTH - 1; i++)
        trail[i] = trail[i + 1];
      trail[BALL_TRAIL_LENGTH - 1].x = (float)rx;
      trail[BALL_TRAIL_LENGTH - 1].y = (float)ry;
    }

    rx += rx_move;
    ry += ry_move;

    // Wall collisions
    if (rx + BALL_SIZE >= BOARD_WIDTH || rx - BALL_SIZE <= 0) reverse_x();
    if (ry - BALL_SIZE <= 0) reverse_y();

    // Paddle collision
    float pw = rozm * paddle_w_mult;
    if (ry + BALL_SIZE >= y && ry + BALL_SIZE < y + rozm &&
        rx + BALL_SIZE >= x && rx - BALL_SIZE <= x + (int)pw &&
        get_ry_move() > 0) {
      reverse_y();
      // Angle the ball based on where it hit the paddle
      float hit_pos = ((float)(rx - x)) / pw;  // 0.0 = left, 1.0 = right
      if (hit_pos < 0.3f)
        rx_move = -speed;
      else if (hit_pos > 0.7f)
        rx_move = speed;
    }

    // Ball fell off bottom
    if (ry > BOARD_HEIGHT) {
      (*lives)--;
      new_game(x, y, rozm);
      *game_running = false;
      trail_count = 0;
    }

    draw_trail();
    draw_ball();
  } else {
    int pw = (int)(rozm * paddle_w_mult);
    rx = x + pw / 2;
    ry = y - BALL_SIZE;
    trail_count = 0;
    draw_ball();
  }
}

void ball::draw_ball() {
  // Outer glow
  al_draw_filled_circle((float)rx, (float)ry, BALL_SIZE + 3,
                        al_map_rgba(255, 80, 20, 40));
  // Main ball
  al_draw_filled_circle((float)rx, (float)ry, BALL_SIZE,
                        al_map_rgba(220, 40, 0, 200));
  // Inner highlight
  al_draw_filled_circle((float)rx - 3, (float)ry - 3, BALL_SIZE * 0.5f,
                        al_map_rgba(255, 180, 120, 120));
}

void ball::draw_trail() {
  for (int i = 0; i < trail_count; i++) {
    float frac = (float)i / (float)trail_count;
    float radius = BALL_SIZE * frac * 0.7f;
    int alpha = (int)(frac * 80);
    al_draw_filled_circle(trail[i].x, trail[i].y, radius,
                          al_map_rgba(255, 60, 10, alpha));
  }
}

// ============================================================
//  Tile
// ============================================================

tile::tile(int x, int y, int w, int h, unsigned char r, unsigned char g,
           unsigned char b, int hitpoints) {
  tile_x = x;
  tile_y = y;
  tile_width = w;
  tile_height = h;
  base_r = r;
  base_g = g;
  base_b = b;
  hp = hitpoints;
  max_hp = hitpoints;
  tile_visible = true;
}

int tile::get_x() { return tile_x; }
int tile::get_y() { return tile_y; }
int tile::get_width() { return tile_width; }
int tile::get_height() { return tile_height; }
int tile::get_hp() { return hp; }
int tile::get_max_hp() { return max_hp; }
bool tile::get_visible() { return tile_visible; }
void tile::set_visible(bool vis) { tile_visible = vis; }
void tile::set_hp(int h) { hp = h; max_hp = h; }

void tile::set_color(unsigned char r, unsigned char g, unsigned char b) {
  base_r = r;
  base_g = g;
  base_b = b;
}

void tile::get_color(unsigned char &r, unsigned char &g, unsigned char &b) {
  r = base_r;
  g = base_g;
  b = base_b;
}

bool tile::hit() {
  hp--;
  if (hp <= 0) {
    tile_visible = false;
    return true;
  }
  return false;
}

void tile::draw_beveled() {
  if (!tile_visible) return;

  float hp_ratio = (float)hp / (float)max_hp;
  // Darken color as HP decreases
  unsigned char r = (unsigned char)(base_r * (0.4f + 0.6f * hp_ratio));
  unsigned char g = (unsigned char)(base_g * (0.4f + 0.6f * hp_ratio));
  unsigned char b = (unsigned char)(base_b * (0.4f + 0.6f * hp_ratio));

  float x1 = (float)tile_x + 1;
  float y1 = (float)tile_y + 1;
  float x2 = (float)(tile_x + tile_width) - 1;
  float y2 = (float)(tile_y + tile_height) - 1;

  // Main body
  al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(r, g, b));

  // Top highlight
  unsigned char hr = (unsigned char)(r + (255 - r) * 0.4f);
  unsigned char hg = (unsigned char)(g + (255 - g) * 0.4f);
  unsigned char hb = (unsigned char)(b + (255 - b) * 0.4f);
  al_draw_filled_rectangle(x1, y1, x2, y1 + 3, al_map_rgb(hr, hg, hb));
  // Left highlight
  al_draw_filled_rectangle(x1, y1, x1 + 3, y2, al_map_rgb(hr, hg, hb));

  // Bottom shadow
  unsigned char sr = (unsigned char)(r * 0.4f);
  unsigned char sg = (unsigned char)(g * 0.4f);
  unsigned char sb = (unsigned char)(b * 0.4f);
  al_draw_filled_rectangle(x1, y2 - 3, x2, y2, al_map_rgb(sr, sg, sb));
  // Right shadow
  al_draw_filled_rectangle(x2 - 3, y1, x2, y2, al_map_rgb(sr, sg, sb));

  // Crack lines for damaged tiles
  if (hp < max_hp) {
    ALLEGRO_COLOR crack_col = al_map_rgba(0, 0, 0, 120);
    float cx = (x1 + x2) / 2.0f;
    float cy = (y1 + y2) / 2.0f;

    if (max_hp - hp >= 1) {
      al_draw_line(cx - 10, cy - 5, cx + 5, cy + 8, crack_col, 1.5f);
      al_draw_line(cx + 5, cy + 8, cx + 12, cy + 2, crack_col, 1.5f);
    }
    if (max_hp - hp >= 2) {
      al_draw_line(cx + 8, cy - 8, cx - 3, cy + 3, crack_col, 1.5f);
      al_draw_line(cx - 3, cy + 3, cx - 12, cy + 10, crack_col, 1.5f);
      al_draw_line(cx - 8, cy - 3, cx + 2, cy - 10, crack_col, 1.0f);
    }
  }

  // HP indicator dots for multi-hit tiles
  if (max_hp > 1) {
    float dot_y = (y1 + y2) / 2.0f;
    float dot_start = (x1 + x2) / 2.0f - (hp - 1) * 4.0f;
    for (int i = 0; i < hp; i++) {
      al_draw_filled_circle(dot_start + i * 8.0f, dot_y, 2.5f,
                            al_map_rgba(255, 255, 255, 180));
    }
  }
}

// ============================================================
//  Tiles container
// ============================================================

tiles::tiles(int board_width, int board_height, ball *gball) {
  game_ball = gball;
  size_width = board_width / TILES_IN_COLUMN;
  size_height = (int)(board_height * 0.4) / TILES_IN_ROW;
  int c = 0;

  for (int i = 0; i < TILES_IN_COLUMN; i++)
    for (int j = 0; j < TILES_IN_ROW; j++) {
      unsigned char r = rand() % 200 + 55;
      unsigned char g = rand() % 200 + 55;
      unsigned char b = rand() % 200 + 55;
      int hp = (rand() % MAX_TILE_HP) + 1;
      game_tiles[c++] = new tile(i * size_width, j * size_height,
                                 size_width, size_height, r, g, b, hp);
    }
}

tiles::~tiles() {
  for (int i = 0; i < TILES_IN_COLUMN * TILES_IN_ROW; i++)
    delete game_tiles[i];
  particles.clear();
  powerups.clear();
}

bool tiles::game_over() {
  for (int i = 0; i < TILES_IN_COLUMN * TILES_IN_ROW; i++)
    if (game_tiles[i]->get_visible()) return false;
  return true;
}

void tiles::new_game() {
  for (int i = 0; i < TILES_IN_COLUMN * TILES_IN_ROW; i++) {
    unsigned char r = rand() % 200 + 55;
    unsigned char g = rand() % 200 + 55;
    unsigned char b = rand() % 200 + 55;
    int hp = (rand() % MAX_TILE_HP) + 1;
    game_tiles[i]->set_visible(true);
    game_tiles[i]->set_color(r, g, b);
    game_tiles[i]->set_hp(hp);
  }
  particles.clear();
  powerups.clear();
}

void tiles::draw_tiles() {
  for (int i = 0; i < TILES_IN_COLUMN * TILES_IN_ROW; i++)
    game_tiles[i]->draw_beveled();
}

// --- Improved particle system ---

void tiles::spawn_particles(int x, int y, int w, int h, unsigned char r,
                            unsigned char g, unsigned char b) {
  float cx = x + w / 2.0f;
  float cy = y + h / 2.0f;
  for (int i = 0; i < PARTICLE_COUNT; i++) {
    particle p;
    p.x = cx + (rand() % w) - w / 2.0f;
    p.y = cy + (rand() % h) - h / 2.0f;
    float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
    float spd = (float)(rand() % 100) / 100.0f * PARTICLE_SPEED + 1.0f;
    p.vx = cosf(angle) * spd;
    p.vy = sinf(angle) * spd - 2.0f;  // slight upward bias
    p.alpha = 1.0f;
    // Slight color variation
    p.r = (unsigned char)(r + rand() % 40 - 20 > 255 ? 255 : r + rand() % 40 - 20);
    p.g = (unsigned char)(g + rand() % 40 - 20 > 255 ? 255 : g + rand() % 40 - 20);
    p.b = (unsigned char)(b + rand() % 40 - 20 > 255 ? 255 : b + rand() % 40 - 20);
    particles.push_back(p);
  }
}

void tiles::update_and_draw_particles() {
  for (int i = (int)particles.size() - 1; i >= 0; i--) {
    particle &p = particles[i];
    p.vy += PARTICLE_GRAVITY;
    p.x += p.vx;
    p.y += p.vy;
    p.alpha -= PARTICLE_FADE;

    if (p.alpha <= 0 || p.y > BOARD_HEIGHT) {
      particles.erase(particles.begin() + i);
      continue;
    }

    int a = (int)(p.alpha * 255);
    float size = 2.0f + p.alpha * 3.0f;
    al_draw_filled_circle(p.x, p.y, size, al_map_rgba(p.r, p.g, p.b, a));
  }
}

// --- Power-ups ---

void tiles::update_powerups() {
  for (int i = (int)powerups.size() - 1; i >= 0; i--) {
    powerups[i].y += POWERUP_SPEED;
    if (powerups[i].y > BOARD_HEIGHT) {
      powerups.erase(powerups.begin() + i);
    }
  }
}

void tiles::draw_powerups(ALLEGRO_FONT *font) {
  for (size_t i = 0; i < powerups.size(); i++) {
    powerup &pu = powerups[i];
    ALLEGRO_COLOR bg, fg;
    const char *label;

    switch (pu.type) {
      case POWERUP_WIDER:
        bg = al_map_rgba(40, 200, 40, 220);
        fg = al_map_rgb(255, 255, 255);
        label = "W";
        break;
      case POWERUP_SLOW:
        bg = al_map_rgba(40, 100, 240, 220);
        fg = al_map_rgb(255, 255, 255);
        label = "S";
        break;
      case POWERUP_LIFE:
        bg = al_map_rgba(230, 40, 40, 220);
        fg = al_map_rgb(255, 255, 255);
        label = "+";
        break;
      default:
        bg = al_map_rgba(200, 200, 200, 220);
        fg = al_map_rgb(0, 0, 0);
        label = "?";
        break;
    }

    float x1 = pu.x;
    float y1 = pu.y;
    float x2 = pu.x + POWERUP_WIDTH;
    float y2 = pu.y + POWERUP_HEIGHT;

    // Rounded-ish pill shape
    al_draw_filled_rectangle(x1, y1, x2, y2, bg);
    al_draw_rectangle(x1, y1, x2, y2, al_map_rgba(255, 255, 255, 150), 1.5f);
    // Pulsing glow
    float glow = 0.5f + 0.5f * sinf(pu.y * 0.05f);
    al_draw_filled_rectangle(x1 + 1, y1 + 1, x2 - 1, y2 - 1,
                             al_map_rgba(255, 255, 255, (int)(glow * 40)));

    if (font)
      al_draw_text(font, fg, (x1 + x2) / 2 - 3, y1 + 1, 0, label);
  }
}

int tiles::collect_powerup(float paddle_x, float paddle_y, float paddle_w,
                           float paddle_h) {
  for (int i = (int)powerups.size() - 1; i >= 0; i--) {
    powerup &pu = powerups[i];
    if (pu.x + POWERUP_WIDTH >= paddle_x &&
        pu.x <= paddle_x + paddle_w &&
        pu.y + POWERUP_HEIGHT >= paddle_y &&
        pu.y <= paddle_y + paddle_h) {
      int type = pu.type;
      powerups.erase(powerups.begin() + i);
      return type;
    }
  }
  return 0;
}

// --- Collision detection ---

int tiles::check_collisions(bool game_running, int *shake) {
  int score = 0;

  for (int i = 0; i < TILES_IN_COLUMN * TILES_IN_ROW; i++) {
    if (!game_tiles[i]->get_visible()) continue;
    if (!game_running) continue;

    int bx = game_ball->get_x();
    int by = game_ball->get_y();
    int tx = game_tiles[i]->get_x();
    int ty = game_tiles[i]->get_y();
    int tw = size_width;
    int th = size_height;
    bool collision = false;

    // Vertical collision
    if (bx + BALL_SIZE >= tx && bx - BALL_SIZE <= tx + tw &&
        ((by - BALL_SIZE <= ty + th && by - BALL_SIZE >= ty) ||
         (by + BALL_SIZE >= ty && by + BALL_SIZE <= ty + th))) {
      game_ball->reverse_y();
      collision = true;
    } else
    // Horizontal collision
    if (by + BALL_SIZE >= ty && by - BALL_SIZE <= ty + th &&
        ((bx - BALL_SIZE <= tx + tw && bx - BALL_SIZE >= tx) ||
         (bx + BALL_SIZE >= tx && bx + BALL_SIZE <= tx + tw))) {
      game_ball->reverse_x();
      collision = true;
    }

    if (collision) {
      score += SCORE_PER_HIT;
      bool destroyed = game_tiles[i]->hit();

      if (destroyed) {
        score += SCORE_PER_DESTROY;
        *shake = SHAKE_FRAMES;

        unsigned char r, g, b;
        game_tiles[i]->get_color(r, g, b);
        spawn_particles(tx, ty, tw, th, r, g, b);

        // Maybe drop a power-up
        if (rand() % 100 < POWERUP_DROP_CHANCE) {
          powerup pu;
          pu.x = (float)(tx + tw / 2 - POWERUP_WIDTH / 2);
          pu.y = (float)(ty + th);
          pu.type = (rand() % POWERUP_TYPE_COUNT) + 1;
          pu.active = true;
          powerups.push_back(pu);
        }
      } else {
        // Small particle burst for hit (not destroy)
        unsigned char r, g, b;
        game_tiles[i]->get_color(r, g, b);
        for (int j = 0; j < 6; j++) {
          particle p;
          p.x = (float)bx;
          p.y = (float)by;
          float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
          p.vx = cosf(angle) * 2.0f;
          p.vy = sinf(angle) * 2.0f;
          p.alpha = 0.7f;
          p.r = r; p.g = g; p.b = b;
          particles.push_back(p);
        }
      }
      break;
    }
  }
  return score;
}
