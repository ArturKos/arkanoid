#include "game_objects.h"

void ball::make_ball_move(int x, int y, int rozm, bool *game_running){
  if(*game_running){
    rx+=rx_move;
    ry+=ry_move;
    if((rx+BALL_SIZE >= BOARD_WIDTH) || (rx-BALL_SIZE <= 0)) reverse_x();
    if((ry+BALL_SIZE >= y && ry+BALL_SIZE < y+5 && rx-BALL_SIZE >= x && rx+BALL_SIZE<=x+3*rozm && get_ry_move()>0)  || (ry-BALL_SIZE <= 0)) reverse_y();
    if(ry > BOARD_HEIGHT) { new_game(x, y, rozm); *game_running = false; }
    draw(rx, ry, BALL_SIZE, al_map_rgba(200,28, 0,60));
  } else{
    rx = x+(rozm*3)/2;
    ry = y-BALL_SIZE;
    draw(x+(rozm*3)/2, y-BALL_SIZE, BALL_SIZE, al_map_rgba(200,28, 0,60));
  }
}

int ball::get_x() { return rx;}
int ball::get_y() { return ry;}
int ball::get_ry_move() { return ry_move;}
void ball::reverse_y() { ry_move = -ry_move;}
void ball::reverse_x() { rx_move = -rx_move;}
void ball::new_game(int x, int y, int rozm) {
  rx = x+(rozm*3)/2;
  ry = y-BALL_SIZE;
  ry_move = -6;
  if (rand()%2 == 1) rx_move = -6; else rx_move = 6;
}

int tile::get_x() {return tile_x;}
int tile::get_y() {return tile_y;}
int tile::get_width() {return tile_width;}
int tile::get_height() {return tile_height;}
ALLEGRO_COLOR  tile::get_color() {return tile_color;}

tiles::tiles(int board_width, int board_height, ball *gball){
  game_ball = gball;
  size_width = board_width/TILES_IN_COLUMN;
  size_height = (board_height*0.4)/TILES_IN_ROW;
  int c = 0;

    for(int i = 0; i < TILES_IN_COLUMN; i++)
     for(int j = 0; j < TILES_IN_ROW; j++){
       game_tiles[c++] = new tile(i*size_width, j*size_height,size_width, size_height,  al_map_rgb(rand()%255,  rand()%255, rand()%255));
     }
}
tiles::~tiles(){
  for(int i = 0; i < TILES_IN_COLUMN*TILES_IN_ROW; i++)
   delete game_tiles[i];
}

bool tiles::game_over(){
 for(int i = 0; i < TILES_IN_COLUMN*TILES_IN_ROW; i++)
  if(game_tiles[i]->get_visible()) return false;
return true;
}

void tiles::new_game(){
  for(int i = 0; i < TILES_IN_COLUMN*TILES_IN_ROW; i++){
    game_tiles[i]->set_visible(true);
    game_tiles[i]->set_color(al_map_rgb(rand()%255,  rand()%255, rand()%255));
  }
}
void tiles::check_collisions(bool game_running){

  for(int i = 0; i < TILES_IN_COLUMN*TILES_IN_ROW; i++)
  if(game_tiles[i]->get_visible())
    draw(game_tiles[i]->get_x(), game_tiles[i]->get_y(), game_tiles[i]->get_x()+size_width, game_tiles[i]->get_y()+size_height, game_tiles[i]->get_color() );
  if(game_running)
  for(int i = 0; i < TILES_IN_COLUMN*TILES_IN_ROW; i++){
    if(game_tiles[i]->get_visible())
    if((game_ball->get_x()+BALL_SIZE <= game_tiles[i]->get_x()+size_width && game_ball->get_x()-BALL_SIZE >= game_tiles[i]->get_x()) && (game_ball->get_y()-BALL_SIZE <= game_tiles[i]->get_y()+size_height) ){
     game_tiles[i]->set_visible(false);
     if(game_ball->get_ry_move()<0)
     game_ball->reverse_y();
     break;
    }
  }
}
