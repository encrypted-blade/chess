#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <optional>
#include <unordered_map>

inline sf::RenderWindow create_window() {
  return sf::RenderWindow{sf::VideoMode{{512, 512}}, "Chess", sf::Style::Close};
}

inline void handle_events(sf::RenderWindow &window) {
  while (const std::optional event{window.pollEvent()}) {
    if (event->is<sf::Event::Closed>()) {
      window.close();
    }
  }
}

constexpr sf::Color dustyGrape{78, 65, 135};
constexpr sf::Color brilliantAzure{48, 131, 220};
constexpr sf::Color lightYellow{248, 255, 229};
constexpr sf::Color lightGreen{125, 222, 146};
constexpr sf::Color oceanMist{46, 191, 165};

enum class piece : char {
  whiterook,
  whiteknight,
  whitebishop,
  whitequeen,
  whiteking,
  whitepawn,
  blackrook,
  blackknight,
  blackbishop,
  blackqueen,
  blackking,
  blackpawn,
  none
};

enum class square : char {
  a1,
  a2,
  a3,
  a4,
  a5,
  a6,
  a7,
  a8,
  b1,
  b2,
  b3,
  b4,
  b5,
  b6,
  b7,
  b8,
  c1,
  c2,
  c3,
  c4,
  c5,
  c6,
  c7,
  c8,
  d1,
  d2,
  d3,
  d4,
  d5,
  d6,
  d7,
  d8,
  e1,
  e2,
  e3,
  e4,
  e5,
  e6,
  e7,
  e8,
  f1,
  f2,
  f3,
  f4,
  f5,
  f6,
  f7,
  f8,
  g1,
  g2,
  g3,
  g4,
  g5,
  g6,
  g7,
  g8,
  h1,
  h2,
  h3,
  h4,
  h5,
  h6,
  h7,
  h8
};

enum class squarecolor : char { white, black };

struct info {
  squarecolor squareColor;
  piece pieceOnSquare;
  unsigned x;
  unsigned y;
};

// Helper: convert square to file (0..7) and rank (0..7) where a1 = file 0, rank
// 0
inline void square_to_file_rank(square s, unsigned &file, unsigned &rank) {
  unsigned idx = static_cast<unsigned>(s);
  // enum is listed by files a..h each with ranks 1..8 so:
  // idx = file*8 + rank where rank 0 == 1, rank 7 == 8
  file = idx / 8;
  rank = idx % 8;
}

// Helper: build a canonical board (pieces + square colors), coordinates for
// white-view (a1 = 0,0)
inline std::unordered_map<square, info>
make_canonical_board(unsigned squareSize = 64) {
  using S = square;
  using P = piece;
  using C = squarecolor;
  std::unordered_map<square, info> board;

  // pieces according to standard start (white on rank1-2, black on 7-8)
  board[S::a1] = {C::black, P::whiterook, 0 * squareSize, 0 * squareSize};
  board[S::b1] = {C::white, P::whiteknight, 1 * squareSize, 0 * squareSize};
  board[S::c1] = {C::black, P::whitebishop, 2 * squareSize, 0 * squareSize};
  board[S::d1] = {C::white, P::whitequeen, 3 * squareSize, 0 * squareSize};
  board[S::e1] = {C::black, P::whiteking, 4 * squareSize, 0 * squareSize};
  board[S::f1] = {C::white, P::whitebishop, 5 * squareSize, 0 * squareSize};
  board[S::g1] = {C::black, P::whiteknight, 6 * squareSize, 0 * squareSize};
  board[S::h1] = {C::white, P::whiterook, 7 * squareSize, 0 * squareSize};

  board[S::a2] = {C::white, P::whitepawn, 0 * squareSize, 1 * squareSize};
  board[S::b2] = {C::black, P::whitepawn, 1 * squareSize, 1 * squareSize};
  board[S::c2] = {C::white, P::whitepawn, 2 * squareSize, 1 * squareSize};
  board[S::d2] = {C::black, P::whitepawn, 3 * squareSize, 1 * squareSize};
  board[S::e2] = {C::white, P::whitepawn, 4 * squareSize, 1 * squareSize};
  board[S::f2] = {C::black, P::whitepawn, 5 * squareSize, 1 * squareSize};
  board[S::g2] = {C::white, P::whitepawn, 6 * squareSize, 1 * squareSize};
  board[S::h2] = {C::black, P::whitepawn, 7 * squareSize, 1 * squareSize};

  // empty ranks 3-6
  for (unsigned r = 2; r <= 5; ++r) {
    for (unsigned f = 0; f < 8; ++f) {
      square s = static_cast<square>(f * 8 + r);
      squarecolor col = ((f + r) % 2 == 0) ? C::black : C::white;
      board[s] = {col, P::none, f * squareSize, r * squareSize};
    }
  }

  board[S::a7] = {C::black, P::blackpawn, 0 * squareSize, 6 * squareSize};
  board[S::b7] = {C::white, P::blackpawn, 1 * squareSize, 6 * squareSize};
  board[S::c7] = {C::black, P::blackpawn, 2 * squareSize, 6 * squareSize};
  board[S::d7] = {C::white, P::blackpawn, 3 * squareSize, 6 * squareSize};
  board[S::e7] = {C::black, P::blackpawn, 4 * squareSize, 6 * squareSize};
  board[S::f7] = {C::white, P::blackpawn, 5 * squareSize, 6 * squareSize};
  board[S::g7] = {C::black, P::blackpawn, 6 * squareSize, 6 * squareSize};
  board[S::h7] = {C::white, P::blackpawn, 7 * squareSize, 6 * squareSize};

  board[S::a8] = {C::white, P::blackrook, 0 * squareSize, 7 * squareSize};
  board[S::b8] = {C::black, P::blackknight, 1 * squareSize, 7 * squareSize};
  board[S::c8] = {C::white, P::blackbishop, 2 * squareSize, 7 * squareSize};
  board[S::d8] = {C::black, P::blackqueen, 3 * squareSize, 7 * squareSize};
  board[S::e8] = {C::white, P::blackking, 4 * squareSize, 7 * squareSize};
  board[S::f8] = {C::black, P::blackbishop, 5 * squareSize, 7 * squareSize};
  board[S::g8] = {C::white, P::blackknight, 6 * squareSize, 7 * squareSize};
  board[S::h8] = {C::black, P::blackrook, 7 * squareSize, 7 * squareSize};

  return board;
}

// Flip function: returns a new map with x,y transformed for the opposite
// perspective
inline std::unordered_map<square, info>
flipped_board(const std::unordered_map<square, info> &src,
              unsigned boardPixelSize = 8 * 64) {
  std::unordered_map<square, info> out;
  unsigned maxIndex =
      boardPixelSize - 64; // top/left origin at 0, last square origin at 7*64

  for (const auto &kv : src) {
    square s = kv.first;
    const info &inf = kv.second;

    // compute file, rank from current x,y (assumes canonical multiples of 64)
    unsigned file = inf.x / 64;
    unsigned rank = inf.y / 64;

    // flipped file/rank
    unsigned file_f = 7 - file;
    unsigned rank_f = 7 - rank;

    info inf2 = inf;
    inf2.x = file_f * 64;
    inf2.y = rank_f * 64;

    out.emplace(s, inf2);
  }
  return out;
}
