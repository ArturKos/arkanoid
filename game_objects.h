typedef void( * drawT )( float, float, float, ALLEGRO_COLOR );
typedef void(* draw_rectT)(float, float, float, float, ALLEGRO_COLOR);


class ball{
private:
  int ball_size, rx, ry, rx_move, ry_move;
  drawT draw;
public:
  ball(int size){
    ball_size = size;
    new_game();
  }
  void new_game();
  int get_x();
  int get_y();
  void set_draw_function(drawT d) {draw = d;}
  void make_ball_move(int x, int y, int rozm);
  void reverse_y();
  void reverse_x();
};

class tile{
private:
  int tile_x, tile_y, tile_width, tile_height;
  ALLEGRO_COLOR  tile_color;
  bool tile_visible;
public:
  tile(int x, int y, int width, int height, ALLEGRO_COLOR color){
    tile_x = x;
    tile_y = y;
    tile_width = width;
    tile_height = height;
    tile_color = color;
    tile_visible = true;
  }
  int get_x();
  int get_y();
  int get_width();
  int get_height();
  void set_visible(bool vis) { tile_visible = vis;}
  bool get_visible() { return tile_visible;}
  ALLEGRO_COLOR get_color();
  void set_color(ALLEGRO_COLOR col) {tile_color = col;}
};

class tiles{
private:
  tile *game_tiles[TILES_IN_COLUMN*TILES_IN_ROW]; //10 kolumn, 5 rzędów
  ball *game_ball;
  draw_rectT draw;
  int size_width;
  int size_height;
public:
  tiles(int board_width, int board_height, ball *gball);
  ~tiles();
  void check_collisions();
  bool game_over();
  void new_game();
  void set_draw_function(draw_rectT d) {draw = d;}
  int get_size_width() {return size_width;}
  int get_size_height() {return size_height;}
};
