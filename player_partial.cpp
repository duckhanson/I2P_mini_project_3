#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#define MIN INT32_MIN
#define MAX INT32_MAX
class Point {
 public:
  int x, y;
  Point() : x(0), y(0) {}
  Point(int x, int y) : x(x), y(y) {}
  bool operator==(const Point& rhs) const { return x == rhs.x && y == rhs.y; }
  bool operator!=(const Point& rhs) const { return !operator==(rhs); }
  Point operator+(const Point& rhs) const {
    return Point(x + rhs.x, y + rhs.y);
  }
  Point operator-(const Point& rhs) const {
    return Point(x - rhs.x, y - rhs.y);
  }
  Point& operator=(const Point& rhs) {
    x = rhs.x;
    y = rhs.y;
    return *this;
  }
};

class OthelloBoard {
 public:
  enum SPOT_STATE { EMPTY = 0, BLACK = 1, WHITE = 2 };
  static const int SIZE = 8;
  const std::array<Point, 8> directions{
      {Point(-1, -1), Point(-1, 0), Point(-1, 1), Point(0, -1),
       /*{0, 0}, */ Point(0, 1), Point(1, -1), Point(1, 0), Point(1, 1)}};
  const std::array<Point, 4> corner_pts{{Point(0, 0), Point(0, SIZE - 1),
                                         Point(SIZE - 1, 0),
                                         Point(SIZE - 1, SIZE - 1)}};
  const std::array<Point, 12> danger_zone{
      {Point(0, 1), Point(1, 0), Point(1, 1), Point(0, SIZE - 2),
       Point(1, SIZE - 1), Point(1, SIZE - 2), Point(SIZE - 1, 1),
       Point(SIZE - 2, 0), Point(SIZE - 2, 1), Point(SIZE - 1, SIZE - 2),
       Point(SIZE - 2, SIZE - 1), Point(SIZE - 2, SIZE - 2)}};
  std::array<std::array<int, SIZE>, SIZE> board;
  std::vector<Point> next_valid_spots;
  std::array<int, 3> disc_count;
  int cur_player, player;

 private:
  int get_next_player(int player) const { return 3 - player; }
  bool is_spot_on_board(Point p) const {
    return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
  }
  int get_disc(Point p) const { return board[p.x][p.y]; }
  void set_disc(Point p, int disc) { board[p.x][p.y] = disc; }
  bool is_disc_at(Point p, int disc) const {
    if (!is_spot_on_board(p)) return false;
    if (get_disc(p) != disc) return false;
    return true;
  }
  bool is_end_of_the_game() const {
    int disc_sum = disc_count[BLACK] + disc_count[WHITE] + disc_count[BLACK];
    if (disc_sum == 64 || next_valid_spots.size() == 0) return true;
    return false;
  }
  bool is_spot_valid(Point center) const {
    if (get_disc(center) != EMPTY) return false;
    for (Point dir : directions) {
      // Move along the direction while testing.
      Point p = center + dir;
      if (!is_disc_at(p, get_next_player(cur_player))) continue;
      p = p + dir;
      while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
        if (is_disc_at(p, cur_player)) return true;
        p = p + dir;
      }
    }
    return false;
  }
  bool is_spot_stable(Point center) const {
    if (get_disc(center) == EMPTY || get_disc(center) == get_next_player(player)) return 0;
    int dir_line = 0;
    Point p;
    for (Point dir : directions) {
      p = center + dir;
      while (is_disc_at(p, player))
        p = p + dir;
      if (!is_spot_on_board(p)) dir_line++;
    }
    if (dir_line > 3 && dir_line < 5)
      return 1;
    else
      return 0;
  }
  void flip_discs(Point center) {
    for (Point dir : directions) {
      // Move along the direction while testing.
      Point p = center + dir;
      if (!is_disc_at(p, get_next_player(cur_player))) continue;
      std::vector<Point> discs({p});
      p = p + dir;
      while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
        if (is_disc_at(p, cur_player)) {
          for (Point s : discs) {
            set_disc(s, cur_player);
          }
          disc_count[cur_player] += discs.size();
          disc_count[get_next_player(cur_player)] -= discs.size();
          break;
        }
        discs.push_back(p);
        p = p + dir;
      }
    }
  }

 public:
  OthelloBoard(std::ifstream& fin, std::ofstream& fout) {
    reset(fin);
    write_valid_spot(fout);
  }
  void reset(std::ifstream& fin) {
    read_board(fin);
    int empty_block = 0, black_block = 0, white_block = 0;
    for (int i = 0; i < SIZE; i++) {
      for (int j = 0; j < SIZE; j++) {
        if (board[i][j] == EMPTY)
          empty_block++;
        else if (board[i][j] == BLACK)
          black_block++;
        else
          white_block++;
      }
    }
    disc_count[EMPTY] = empty_block;
    disc_count[BLACK] = black_block;
    disc_count[WHITE] = white_block;
    read_valid_spots(fin);
  }
  void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
      for (int j = 0; j < SIZE; j++) {
        fin >> board[i][j];
      }
    }
  }
  void read_valid_spots(std::ifstream& fin) {
    size_t n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (size_t i = 0; i < n_valid_spots; i++) {
      fin >> x >> y;
      next_valid_spots.push_back(Point(x, y));
    }
  }
  std::vector<Point> get_valid_spots() const {
    std::vector<Point> valid_spots;
    for (int i = 0; i < SIZE; i++) {
      for (int j = 0; j < SIZE; j++) {
        Point p = Point(i, j);
        if (board[i][j] != EMPTY) continue;
        if (is_spot_valid(p)) valid_spots.push_back(p);
      }
    }
    return valid_spots;
  }
  void put_disc(Point p) {
    set_disc(p, cur_player);
    disc_count[cur_player]++;
    disc_count[EMPTY]--;
    flip_discs(p);
    give_control_to_next_player();
  }
  void restore_disc(Point p) {
    set_disc(p, EMPTY);
    disc_count[cur_player]++;
    disc_count[EMPTY]--;
    give_control_to_next_player();
  }
  void give_control_to_next_player() {
    // Give control to the other player.
    cur_player = get_next_player(cur_player);
  }
  int get_corner() {
    int corner = 0;
    int disc;
    for (auto c : corner_pts) {
      disc = get_disc(c);
      if (disc == player) corner++;
      else if (disc == get_next_player(player))
        corner--;
    }
    return corner;
  }
  int get_stable_spots() {
    int s = 0;
    for (size_t i = 0; i < SIZE; i++) {
      for (size_t j = 0; j < SIZE; j++) {
        s += is_spot_stable(Point(i, j));
      }
    }
    return s;
  }
  int get_danger_zone() {
    int danger = 0;
    int zone_count = 0;
    int disc;
    for (auto d : danger_zone) {
      disc = get_disc(d);
      zone_count++;
      if (disc == player) {
        if (is_disc_at(corner_pts[zone_count / 3], player)) {
            danger--;
        } else {
          if (zone_count % 3 == 0)
            danger += 10;
          else
            danger++;
        }
      }
    }
    return danger;
  }
  int heuristic() {
    size_t alpha_on_the_board = disc_count[player];
    return alpha_on_the_board;
  }

  int alpha_beta(Point p, int depth, int alpha, int beta, bool maxPlayer) {
    // maxPlayer is player who wants to win the game.
    put_disc(p);
    int cur_size = next_valid_spots.size();
    auto next_valid_points = get_valid_spots();
    auto copy_board = board;
    int value;
    // End of alpha beta.
    if (depth == 0 || cur_size == 1 || is_end_of_the_game()) {
      value = heuristic();
      restore_disc(p);
      board = copy_board;
      return value;
    }
    int optimize_depth;
    optimize_depth = depth - 1;

    if (maxPlayer) {
      value = MIN;
      for (auto np : next_valid_points) {
        value =
            std::max(value, alpha_beta(np, optimize_depth, alpha, beta, false));
        alpha = std::max(alpha, value);
        if (alpha >= beta) break;
      }
    } else {
      value = MAX;
      for (auto np : next_valid_points) {
        value =
            std::min(value, alpha_beta(np, optimize_depth, alpha, beta, true));
        beta = std::min(beta, value);
        if (alpha >= beta) break;
      }
    }
    // restore conditions
    if (!next_valid_points.empty()) next_valid_points.clear();
    restore_disc(p);
    board = copy_board;
    return value;
  }

  void write_valid_spot(std::ofstream& fout) {
    int value, child;
    int alpha, beta;
    Point p;
    value = MIN, child = MIN, alpha = MIN, beta = MAX;
    if (next_valid_spots.size() == 1)
      p = next_valid_spots[0];
    else {
      for (auto np : next_valid_spots) {
        // max_player's point of view
        // race between depth(4) and depth(5), 4 wins always.
        if (next_valid_spots.size() < 13)
          child = alpha_beta(np, 5, alpha, beta, false);
        else
          child = alpha_beta(np, 4, alpha, beta, false);
        alpha = std::max(alpha, child);
        if (child > value) {
          value = child;
          p = np;
          fout << np.x << " " << np.y << std::endl;
          fout.flush();
        }
      }
    }
    // Keep updating the output until getting killed.
    size_t count = 100;
    while (count--) {
      fout << p.x << " " << p.y << std::endl;
      fout.flush();
    }
  }
};

int main(int, char** argv) {
  std::ifstream fin(argv[1]);
  std::ofstream fout(argv[2]);
  OthelloBoard B(fin, fout);
  fin.close();
  fout.close();
  return 0;
}
