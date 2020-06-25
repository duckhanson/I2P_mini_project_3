#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#define MIN INT32_MIN
#define MAX INT32_MAX
struct Point {
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
};

int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;
std::vector<Point> alpha_next_valid_points;
std::vector<Point> beta_next_valid_points;
const std::array<Point, 8> directions{
    {Point(-1, -1), Point(-1, 0), Point(-1, 1), Point(0, -1),
     /*{0, 0}, */ Point(0, 1), Point(1, -1), Point(1, 0), Point(1, 1)}};

bool is_point_on_board(Point p) {
  return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
}

bool is_last_point() {
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      if (board[i][j] == 0) return false;
    }
  }
  return true;
}

bool is_point_valid(Point& center, bool alpha) {
  if (board[center.x][center.y] != 0) return false;

  int cur_player;
  if (alpha)
    cur_player = player;
  else
    cur_player = 3 - player;

  int nxt_player = 3 - cur_player;

  for (Point dir : directions) {
    // Move along the direction while testing.
    Point p = center + dir;
    if (!(board[p.x][p.y] == nxt_player)) continue;
    p = p + dir;
    while (is_point_on_board(p) && board[p.x][p.y] != 0) {
      if (board[p.x][p.y] == cur_player) return true;
      p = p + dir;
    }
  }
  return false;
}

void get_valid_points(bool alpha) {
  if (alpha) {
    for (int i = 0; i < SIZE; i++) {
      for (int j = 0; j < SIZE; j++) {
        if (board[i][j] != 0) continue;
        Point p = Point(i, j);
        if (is_point_valid(p, true)) alpha_next_valid_points.push_back(p);
      }
    }
  } else {
    for (int i = 0; i < SIZE; i++) {
      for (int j = 0; j < SIZE; j++) {
        if (board[i][j] != 0) continue;
        Point p = Point(i, j);
        if (is_point_valid(p, false)) beta_next_valid_points.push_back(p);
      }
    }
  }
}

int heuristic() {
  int alpha_option, beta_option;
  int alpha_on_the_board = 0;
  int beta_on_the_board = 0;

  get_valid_points(true);
  alpha_option = alpha_next_valid_points.size();

  get_valid_points(false);
  beta_option = beta_next_valid_points.size();

  for (int i = 0; i < SIZE; i++)
    for (int j = 0; j < SIZE; j++)
      if (board[i][j] == player) alpha_on_the_board++;
      else if (board[i][j] == 3 - player) beta_on_the_board++;

  return alpha_option * 0 - beta_option * 1 + (alpha_on_the_board * 3 - beta_on_the_board * 1);
}

int alpha_beta(Point p, int depth, int alpha, int beta, bool maxPlayer) {
  board[p.x][p.y] = player;
  int value;
  if (depth == 0 || is_last_point()) {
    value = heuristic();
    board[p.x][p.y] = 0;
    return value;
  }

  if (maxPlayer) {
    value = MIN;
    get_valid_points(true);  // get valid for alpha player
    for (auto np : alpha_next_valid_points) {
      value = std::max(value, alpha_beta(np, depth - 1, alpha, beta, false));
      alpha = std::max(alpha, value);
      if (alpha >= beta) break;
    }
  } else {
    value = MAX;
    get_valid_points(false);  // get valid for beta player
    for (auto np : beta_next_valid_points) {
      value = std::min(value, alpha_beta(np, depth - 1, alpha, beta, true));
      beta = std::min(beta, value);
      if (alpha >= beta) break;
    }
  }
  alpha_next_valid_points.clear();
  beta_next_valid_points.clear();
  board[p.x][p.y] = 0;
  return value;
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
  int n_valid_spots;
  fin >> n_valid_spots;
  int x, y;
  for (int i = 0; i < n_valid_spots; i++) {
    fin >> x >> y;
    next_valid_spots.push_back({x, y});
  }
}

void write_valid_spot(std::ofstream& fout) {
  int value, child;
  int alpha, beta;
  Point p = next_valid_spots[0];
  value = MIN, child = MIN, alpha = MIN, beta = MAX;
  for (auto np : next_valid_spots) {
    // max_player's point of view
    // race between depth(4) and depth(5), 4 wins always.
    child = alpha_beta(np, 4, alpha, beta, false);
    // std::cout << child << std::endl;
    if (child > value) {
      value = child;
      p = np;
      fout << np.x << " " << np.y << std::endl;
      fout.flush();
    }
  }
  // Keep updating the output until getting killed.
  while (true) {
    fout << p.x << " " << p.y << std::endl;
    fout.flush();
  }
}

int main(int, char** argv) {
  std::ifstream fin(argv[1]);
  std::ofstream fout(argv[2]);
  read_board(fin);
  read_valid_spots(fin);
  write_valid_spot(fout);
  fin.close();
  fout.close();
  return 0;
}
