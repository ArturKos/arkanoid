#ifndef ARKANOID_H
#define ARKANOID_H

// Board
#define BOARD_WIDTH 1024
#define BOARD_HEIGHT 768
#define BACKGROUND_FILE "background.png"

// Ball
#define BALL_SIZE 15
#define BALL_SPEED 6
#define BALL_TRAIL_LENGTH 12

// Tiles
#define TILES_IN_COLUMN 10
#define TILES_IN_ROW 5
#define NUMBER_OF_TILES (TILES_IN_COLUMN * TILES_IN_ROW)
#define MAX_TILE_HP 3

// Particles
#define PARTICLE_COUNT 24
#define PARTICLE_GRAVITY 0.15f
#define PARTICLE_FADE 0.012f
#define PARTICLE_SPEED 4.0f

// Power-ups
#define POWERUP_DROP_CHANCE 25
#define POWERUP_WIDTH 30
#define POWERUP_HEIGHT 14
#define POWERUP_SPEED 2.5f
#define POWERUP_WIDER  1
#define POWERUP_SLOW   2
#define POWERUP_LIFE   3
#define POWERUP_TYPE_COUNT 3
#define POWERUP_DURATION 600

// Paddle
#define PADDLE_SPEED 10
#define PADDLE_WIDTH_MULT 3
#define PADDLE_WIDER_MULT 4.5f

// Game
#define INITIAL_LIVES 3
#define SCORE_PER_HIT 10
#define SCORE_PER_DESTROY 50

// Screen shake
#define SHAKE_FRAMES 10
#define SHAKE_INTENSITY 5

#endif
