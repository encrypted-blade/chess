// TODO: Fix Piece Movement Error

#pragma once
#include "chess_pieces.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <algorithm>
#include <array>
#include <functional>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <optional>
#include <random>
#include <string>
#include <thread>
#include <vector>

constexpr size_t chess_pieces_size = 24495;

// ============= COLORS =============
constexpr sf::Color DUSTY_GRAPE{78, 65, 135};
constexpr sf::Color BRILLIANT_AZURE{48, 131, 220};
constexpr sf::Color LIGHT_YELLOW{248, 255, 229};
constexpr sf::Color LIGHT_GREEN{125, 222, 146};
constexpr sf::Color OCEAN_MIST{46, 191, 165};
constexpr sf::Color SELECTION_COLOR{100, 200, 100};
constexpr sf::Color HIGHLIGHT_COLOR{150, 255, 150};
constexpr sf::Color VALID_MOVE_COLOR{100, 100, 255, 128};
constexpr sf::Color CHECK_COLOR{255, 100, 100};
constexpr sf::Color BUTTON_COLOR{60, 60, 80};
constexpr sf::Color BUTTON_HOVER_COLOR{80, 80, 110};
constexpr sf::Color BUTTON_TEXT_COLOR{255, 255, 255};
constexpr sf::Color TITLE_COLOR{255, 255, 255};

// ============= PIECE ENUMERATION =============
enum class Piece : std::uint8_t {
  WHITE_KING,
  WHITE_QUEEN,
  WHITE_BISHOP,
  WHITE_KNIGHT,
  WHITE_ROOK,
  WHITE_PAWN,
  BLACK_KING,
  BLACK_QUEEN,
  BLACK_BISHOP,
  BLACK_KNIGHT,
  BLACK_ROOK,
  BLACK_PAWN,
  NONE
};

// ============= BITBOARD TYPES & CONSTANTS =============
using u64 = uint64_t;
using Move = uint32_t;

constexpr u64 FILE_A = 0x0101010101010101ULL;
constexpr u64 FILE_H = 0x8080808080808080ULL;
constexpr u64 RANK_1 = 0x00000000000000FFULL;
constexpr u64 RANK_2 = 0x000000000000FF00ULL;
constexpr u64 RANK_3 = 0x0000000000FF0000ULL;
constexpr u64 RANK_6 = 0x0000FF0000000000ULL;
constexpr u64 RANK_7 = 0x00FF000000000000ULL;
constexpr u64 RANK_8 = 0xFF00000000000000ULL;

inline Move make_move(int from, int to, int flags = 0) {
  return from | (to << 6) | (flags << 12);
}
inline int from_sq(Move m) { return m & 0x3F; }
inline int to_sq(Move m) { return (m >> 6) & 0x3F; }
inline int flags(Move m) { return (m >> 12) & 0xF; }
inline int move_score(Move m) { return m >> 16; }
inline void add_move_score(Move &m, int score) { m |= (score << 16); }

// ============= BITBOARD ATTACK TABLES =============
u64 knight_attacks[64];
u64 king_attacks[64];
u64 pawn_attacks[2][64];
u64 rays[64][8]; // 0:N, 1:S, 2:E, 3:W, 4:NE, 5:NW, 6:SE, 7:SW

void init_bitboards() {
  for (int sq = 0; sq < 64; ++sq) {
    int r = sq / 8, f = sq % 8;
    int n_moves[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                         {1, -2},  {1, 2},  {2, -1},  {2, 1}};
    for (auto &m : n_moves) {
      int tr = r + m[0], tf = f + m[1];
      if (tr >= 0 && tr < 8 && tf >= 0 && tf < 8)
        knight_attacks[sq] |= 1ULL << (tr * 8 + tf);
    }
    for (int dr = -1; dr <= 1; ++dr) {
      for (int df = -1; df <= 1; ++df) {
        if (dr == 0 && df == 0)
          continue;
        int tr = r + dr, tf = f + df;
        if (tr >= 0 && tr < 8 && tf >= 0 && tf < 8)
          king_attacks[sq] |= 1ULL << (tr * 8 + tf);
      }
    }
    if (r < 7) {
      if (f > 0)
        pawn_attacks[0][sq] |= 1ULL << ((r + 1) * 8 + (f - 1));
      if (f < 7)
        pawn_attacks[0][sq] |= 1ULL << ((r + 1) * 8 + (f + 1));
    }
    if (r > 0) {
      if (f > 0)
        pawn_attacks[1][sq] |= 1ULL << ((r - 1) * 8 + (f - 1));
      if (f < 7)
        pawn_attacks[1][sq] |= 1ULL << ((r - 1) * 8 + (f + 1));
    }
    int dirs[8][2] = {{1, 0}, {-1, 0}, {0, 1},  {0, -1},
                      {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    for (int d = 0; d < 8; ++d) {
      int tr = r + dirs[d][0], tf = f + dirs[d][1];
      while (tr >= 0 && tr < 8 && tf >= 0 && tf < 8) {
        rays[sq][d] |= 1ULL << (tr * 8 + tf);
        tr += dirs[d][0];
        tf += dirs[d][1];
      }
    }
  }
}

inline u64 get_rook_attacks(int sq, u64 occ) {
  u64 attacks = 0;
  u64 r = rays[sq][0];
  u64 b = r & occ;
  if (b)
    r ^= rays[__builtin_ctzll(b)][0];
  attacks |= r;
  r = rays[sq][1];
  b = r & occ;
  if (b)
    r ^= rays[63 - __builtin_clzll(b)][1];
  attacks |= r;
  r = rays[sq][2];
  b = r & occ;
  if (b)
    r ^= rays[__builtin_ctzll(b)][2];
  attacks |= r;
  r = rays[sq][3];
  b = r & occ;
  if (b)
    r ^= rays[63 - __builtin_clzll(b)][3];
  attacks |= r;
  return attacks;
}
inline u64 get_bishop_attacks(int sq, u64 occ) {
  u64 attacks = 0;
  u64 r = rays[sq][4];
  u64 b = r & occ;
  if (b)
    r ^= rays[__builtin_ctzll(b)][4];
  attacks |= r;
  r = rays[sq][5];
  b = r & occ;
  if (b)
    r ^= rays[63 - __builtin_clzll(b)][5];
  attacks |= r;
  r = rays[sq][6];
  b = r & occ;
  if (b)
    r ^= rays[__builtin_ctzll(b)][6];
  attacks |= r;
  r = rays[sq][7];
  b = r & occ;
  if (b)
    r ^= rays[63 - __builtin_clzll(b)][7];
  attacks |= r;
  return attacks;
}
inline u64 get_queen_attacks(int sq, u64 occ) {
  return get_rook_attacks(sq, occ) | get_bishop_attacks(sq, occ);
}

// ============= BOARD REPRESENTATION =============
struct Position {
  u64 bb[6];     // 0:P, 1:N, 2:B, 3:R, 4:Q, 5:K
  u64 colors[2]; // 0:White, 1:Black
  u64 occ;
  bool white_to_move;
  std::uint8_t ep_square;
  std::uint8_t castling; // 1:WK, 2:WQ, 4:BK, 8:BQ
  std::uint8_t halfmove;

  inline u64 attackers_to(int sq, u64 occ) const {
    return (pawn_attacks[1][sq] & bb[0] & colors[0]) |
           (pawn_attacks[0][sq] & bb[0] & colors[1]) |
           (knight_attacks[sq] & bb[1]) |
           (get_bishop_attacks(sq, occ) & (bb[2] | bb[4])) |
           (get_rook_attacks(sq, occ) & (bb[3] | bb[4])) |
           (king_attacks[sq] & bb[5]);
  }
  inline bool is_attacked(int sq, bool by_white) const {
    u64 attackers = attackers_to(sq, occ);
    return by_white ? (attackers & colors[0]) : (attackers & colors[1]);
  }
};

struct GameState {
  std::optional<std::uint8_t> selected_square;
  std::optional<std::uint8_t> last_from;
  std::optional<std::uint8_t> last_to;
  bool in_check = false;
  bool player_is_white = false;
  bool game_over = false;
  bool restart_requested = false;
  std::string result_message = "";
};

// ============= BOARD INITIALIZATION =============
inline Position make_initial_position() {
  Position pos{};
  pos.bb[0] = 0x000000000000FF00ULL | 0x00FF000000000000ULL;
  pos.bb[1] = 0x0000000000000042ULL | 0x4200000000000000ULL;
  pos.bb[2] = 0x0000000000000024ULL | 0x2400000000000000ULL;
  pos.bb[3] = 0x0000000000000081ULL | 0x8100000000000000ULL;
  pos.bb[4] = 0x0000000000000008ULL | 0x0800000000000000ULL;
  pos.bb[5] = 0x0000000000000010ULL | 0x1000000000000000ULL;
  pos.colors[0] = 0x000000000000FFFFULL;
  pos.colors[1] = 0xFFFF000000000000ULL;
  pos.occ = pos.colors[0] | pos.colors[1];
  pos.white_to_move = true;
  pos.ep_square = 64;
  pos.castling = 15;
  pos.halfmove = 0;
  return pos;
}

// ============= UI HELPERS =============
inline Piece get_piece_at(const Position &pos, int sq) {
  u64 bb = 1ULL << sq;
  if (!(pos.occ & bb))
    return Piece::NONE;
  int pc = 0;
  for (int i = 0; i < 6; ++i)
    if (pos.bb[i] & bb) {
      pc = i;
      break;
    }
  bool is_w = (pos.colors[0] & bb) != 0;
  static const Piece map_w[6] = {Piece::WHITE_PAWN,   Piece::WHITE_KNIGHT,
                                 Piece::WHITE_BISHOP, Piece::WHITE_ROOK,
                                 Piece::WHITE_QUEEN,  Piece::WHITE_KING};
  static const Piece map_b[6] = {Piece::BLACK_PAWN,   Piece::BLACK_KNIGHT,
                                 Piece::BLACK_BISHOP, Piece::BLACK_ROOK,
                                 Piece::BLACK_QUEEN,  Piece::BLACK_KING};
  return is_w ? map_w[pc] : map_b[pc];
}

inline bool is_white_piece(Piece p) noexcept {
  return p != Piece::NONE && static_cast<std::uint8_t>(p) < 6;
}
inline sf::IntRect get_texture_rect(Piece piece) noexcept {
  constexpr int PIECE_SIZE = 64;
  if (piece == Piece::NONE)
    return {{0, 0}, {0, 0}};
  std::uint8_t idx = static_cast<std::uint8_t>(piece);
  bool is_black = idx >= 6;
  return {{(idx % 6) * PIECE_SIZE, static_cast<int>(is_black) * PIECE_SIZE},
          {PIECE_SIZE, PIECE_SIZE}};
}

inline std::uint8_t display_rank_to_board_rank(std::uint8_t display_rank,
                                               bool player_is_white) noexcept {
  return player_is_white ? (7 - display_rank) : display_rank;
}
inline std::uint8_t board_rank_to_display_rank(std::uint8_t board_rank,
                                               bool player_is_white) noexcept {
  return player_is_white ? (7 - board_rank) : board_rank;
}
inline std::optional<std::uint8_t>
pixel_to_board_index(int px, int py, bool player_is_white) noexcept {
  constexpr float SQ = 64.0f, MARGIN = 32.0f, END = MARGIN + 8.0f * SQ;
  if (px < MARGIN || py < MARGIN || px >= END || py >= END)
    return std::nullopt;
  std::uint8_t f = static_cast<std::uint8_t>((px - MARGIN) / SQ);
  std::uint8_t dr = static_cast<std::uint8_t>((py - MARGIN) / SQ);
  if (f >= 8 || dr >= 8)
    return std::nullopt;
  return display_rank_to_board_rank(dr, player_is_white) * 8 + f;
}
inline sf::Vector2f board_index_to_pixel(std::uint8_t idx,
                                         bool player_is_white) noexcept {
  constexpr float SQ = 64.0f, MARGIN = 32.0f;
  return {
      static_cast<float>((idx % 8) * SQ + MARGIN),
      static_cast<float>(
          board_rank_to_display_rank(idx / 8, player_is_white) * SQ + MARGIN)};
}

// ============= MOVE GENERATION =============
struct MoveList {
  Move moves[256];
  int count = 0;
  void add(Move m) { moves[count++] = m; }
};

void generate_pseudo_legal_moves(const Position &pos, MoveList &list) {
  int us = pos.white_to_move ? 0 : 1;
  int them = 1 - us;
  u64 my_pieces = pos.colors[us];
  u64 their_pieces = pos.colors[them];
  u64 empty = ~pos.occ;

  u64 pawns = pos.bb[0] & my_pieces;
  if (us == 0) {
    u64 single_push = (pawns << 8) & empty;
    u64 double_push = ((single_push & RANK_3) << 8) & empty;
    u64 cap_west = (pawns << 7) & their_pieces & ~FILE_H;
    u64 cap_east = (pawns << 9) & their_pieces & ~FILE_A;

    u64 promo_single = single_push & RANK_8;
    u64 normal_single = single_push & ~RANK_8;
    u64 promo_cap_west = cap_west & RANK_8;
    u64 normal_cap_west = cap_west & ~RANK_8;
    u64 promo_cap_east = cap_east & RANK_8;
    u64 normal_cap_east = cap_east & ~RANK_8;

    while (normal_single) {
      int to = __builtin_ctzll(normal_single);
      list.add(make_move(to - 8, to));
      normal_single &= normal_single - 1;
    }
    while (double_push) {
      int to = __builtin_ctzll(double_push);
      list.add(make_move(to - 16, to));
      double_push &= double_push - 1;
    }
    while (normal_cap_west) {
      int to = __builtin_ctzll(normal_cap_west);
      list.add(make_move(to - 7, to));
      normal_cap_west &= normal_cap_west - 1;
    }
    while (normal_cap_east) {
      int to = __builtin_ctzll(normal_cap_east);
      list.add(make_move(to - 9, to));
      normal_cap_east &= normal_cap_east - 1;
    }

    while (promo_single) {
      int to = __builtin_ctzll(promo_single);
      int from = to - 8;
      list.add(make_move(from, to, 6));
      list.add(make_move(from, to, 5));
      list.add(make_move(from, to, 4));
      list.add(make_move(from, to, 3));
      promo_single &= promo_single - 1;
    }
    while (promo_cap_west) {
      int to = __builtin_ctzll(promo_cap_west);
      int from = to - 7;
      list.add(make_move(from, to, 6));
      list.add(make_move(from, to, 5));
      list.add(make_move(from, to, 4));
      list.add(make_move(from, to, 3));
      promo_cap_west &= promo_cap_west - 1;
    }
    while (promo_cap_east) {
      int to = __builtin_ctzll(promo_cap_east);
      int from = to - 9;
      list.add(make_move(from, to, 6));
      list.add(make_move(from, to, 5));
      list.add(make_move(from, to, 4));
      list.add(make_move(from, to, 3));
      promo_cap_east &= promo_cap_east - 1;
    }

    if (pos.ep_square != 64) {
      u64 ep_pawns = pawn_attacks[1][pos.ep_square] & pawns;
      while (ep_pawns) {
        int from = __builtin_ctzll(ep_pawns);
        list.add(make_move(from, pos.ep_square, 1));
        ep_pawns &= ep_pawns - 1;
      }
    }
  } else {
    u64 single_push = (pawns >> 8) & empty;
    u64 double_push = ((single_push & RANK_6) >> 8) & empty;
    u64 cap_west = (pawns >> 9) & their_pieces & ~FILE_H;
    u64 cap_east = (pawns >> 7) & their_pieces & ~FILE_A;

    u64 promo_single = single_push & RANK_1;
    u64 normal_single = single_push & ~RANK_1;
    u64 promo_cap_west = cap_west & RANK_1;
    u64 normal_cap_west = cap_west & ~RANK_1;
    u64 promo_cap_east = cap_east & RANK_1;
    u64 normal_cap_east = cap_east & ~RANK_1;

    while (normal_single) {
      int to = __builtin_ctzll(normal_single);
      list.add(make_move(to + 8, to));
      normal_single &= normal_single - 1;
    }
    while (double_push) {
      int to = __builtin_ctzll(double_push);
      list.add(make_move(to + 16, to));
      double_push &= double_push - 1;
    }
    while (normal_cap_west) {
      int to = __builtin_ctzll(normal_cap_west);
      list.add(make_move(to + 9, to));
      normal_cap_west &= normal_cap_west - 1;
    }
    while (normal_cap_east) {
      int to = __builtin_ctzll(normal_cap_east);
      list.add(make_move(to + 7, to));
      normal_cap_east &= normal_cap_east - 1;
    }

    while (promo_single) {
      int to = __builtin_ctzll(promo_single);
      int from = to + 8;
      list.add(make_move(from, to, 6));
      list.add(make_move(from, to, 5));
      list.add(make_move(from, to, 4));
      list.add(make_move(from, to, 3));
      promo_single &= promo_single - 1;
    }
    while (promo_cap_west) {
      int to = __builtin_ctzll(promo_cap_west);
      int from = to + 9;
      list.add(make_move(from, to, 6));
      list.add(make_move(from, to, 5));
      list.add(make_move(from, to, 4));
      list.add(make_move(from, to, 3));
      promo_cap_west &= promo_cap_west - 1;
    }
    while (promo_cap_east) {
      int to = __builtin_ctzll(promo_cap_east);
      int from = to + 7;
      list.add(make_move(from, to, 6));
      list.add(make_move(from, to, 5));
      list.add(make_move(from, to, 4));
      list.add(make_move(from, to, 3));
      promo_cap_east &= promo_cap_east - 1;
    }

    if (pos.ep_square != 64) {
      u64 ep_pawns = pawn_attacks[0][pos.ep_square] & pawns;
      while (ep_pawns) {
        int from = __builtin_ctzll(ep_pawns);
        list.add(make_move(from, pos.ep_square, 1));
        ep_pawns &= ep_pawns - 1;
      }
    }
  }

  u64 knights = pos.bb[1] & my_pieces;
  while (knights) {
    int from = __builtin_ctzll(knights);
    u64 attacks = knight_attacks[from] & ~my_pieces;
    while (attacks) {
      int to = __builtin_ctzll(attacks);
      list.add(make_move(from, to));
      attacks &= attacks - 1;
    }
    knights &= knights - 1;
  }
  u64 bishops = pos.bb[2] & my_pieces;
  while (bishops) {
    int from = __builtin_ctzll(bishops);
    u64 attacks = get_bishop_attacks(from, pos.occ) & ~my_pieces;
    while (attacks) {
      int to = __builtin_ctzll(attacks);
      list.add(make_move(from, to));
      attacks &= attacks - 1;
    }
    bishops &= bishops - 1;
  }
  u64 rooks = pos.bb[3] & my_pieces;
  while (rooks) {
    int from = __builtin_ctzll(rooks);
    u64 attacks = get_rook_attacks(from, pos.occ) & ~my_pieces;
    while (attacks) {
      int to = __builtin_ctzll(attacks);
      list.add(make_move(from, to));
      attacks &= attacks - 1;
    }
    rooks &= rooks - 1;
  }
  u64 queens = pos.bb[4] & my_pieces;
  while (queens) {
    int from = __builtin_ctzll(queens);
    u64 attacks = get_queen_attacks(from, pos.occ) & ~my_pieces;
    while (attacks) {
      int to = __builtin_ctzll(attacks);
      list.add(make_move(from, to));
      attacks &= attacks - 1;
    }
    queens &= queens - 1;
  }
  u64 kings = pos.bb[5] & my_pieces;
  if (kings) {
    int from = __builtin_ctzll(kings);
    u64 attacks = king_attacks[from] & ~my_pieces;
    while (attacks) {
      int to = __builtin_ctzll(attacks);
      list.add(make_move(from, to));
      attacks &= attacks - 1;
    }

    if (us == 0) {
      if ((pos.castling & 1) && !(pos.occ & 0x60ULL) &&
          !pos.is_attacked(4, true) && !pos.is_attacked(5, true) &&
          !pos.is_attacked(6, true))
        list.add(make_move(4, 6, 2));
      if ((pos.castling & 2) && !(pos.occ & 0xEULL) &&
          !pos.is_attacked(4, true) && !pos.is_attacked(3, true) &&
          !pos.is_attacked(2, true))
        list.add(make_move(4, 2, 2));
    } else {
      if ((pos.castling & 4) && !(pos.occ & 0x6000000000000000ULL) &&
          !pos.is_attacked(60, false) && !pos.is_attacked(61, false) &&
          !pos.is_attacked(62, false))
        list.add(make_move(60, 62, 2));
      if ((pos.castling & 8) && !(pos.occ & 0x0E00000000000000ULL) &&
          !pos.is_attacked(60, false) && !pos.is_attacked(59, false) &&
          !pos.is_attacked(58, false))
        list.add(make_move(60, 58, 2));
    }
  }
}

inline bool is_legal(const Position &pos, Move m) {
  int us = pos.white_to_move ? 0 : 1;
  int from = from_sq(m);
  int to = to_sq(m);
  int f = flags(m);

  Position temp = pos;
  u64 from_bb = 1ULL << from;
  u64 to_bb = 1ULL << to;

  int pc = 0;
  for (int i = 0; i < 6; ++i)
    if (temp.bb[i] & from_bb) {
      pc = i;
      break;
    }

  temp.bb[pc] &= ~from_bb;
  temp.colors[us] &= ~from_bb;

  u64 cap_bb = to_bb & temp.occ;
  if (cap_bb) {
    for (int i = 0; i < 6; ++i)
      temp.bb[i] &= ~to_bb;
    temp.colors[1 - us] &= ~to_bb;
  }

  if (f == 1) {
    u64 ep_cap = us == 0 ? (to_bb >> 8) : (to_bb << 8);
    temp.bb[0] &= ~ep_cap;
    temp.colors[1 - us] &= ~ep_cap;
  }

  if (f == 2) {
    if (to == 6) {
      temp.bb[3] &= ~0x80ULL;
      temp.bb[3] |= 0x20ULL;
      temp.colors[us] &= ~0x80ULL;
      temp.colors[us] |= 0x20ULL;
    }
    if (to == 2) {
      temp.bb[3] &= ~0x01ULL;
      temp.bb[3] |= 0x08ULL;
      temp.colors[us] &= ~0x01ULL;
      temp.colors[us] |= 0x08ULL;
    }
    if (to == 62) {
      temp.bb[3] &= ~0x8000000000000000ULL;
      temp.bb[3] |= 0x2000000000000000ULL;
      temp.colors[us] &= ~0x8000000000000000ULL;
      temp.colors[us] |= 0x2000000000000000ULL;
    }
    if (to == 58) {
      temp.bb[3] &= ~0x0100000000000000ULL;
      temp.bb[3] |= 0x0800000000000000ULL;
      temp.colors[us] &= ~0x0100000000000000ULL;
      temp.colors[us] |= 0x0800000000000000ULL;
    }
  }

  int place_pc = pc;
  if (f >= 3)
    place_pc = 6 - f;

  temp.bb[place_pc] |= to_bb;
  temp.colors[us] |= to_bb;
  temp.occ = temp.colors[0] | temp.colors[1];

  int king_sq = __builtin_ctzll(temp.bb[5] & temp.colors[us]);
  return !temp.is_attacked(king_sq, 1 - us);
}

inline void apply_move(Position &pos, Move m, GameState &state) {
  int us = pos.white_to_move ? 0 : 1;
  int them = 1 - us;
  int from = from_sq(m);
  int to = to_sq(m);
  int f = flags(m);

  state.last_from = from;
  state.last_to = to;

  u64 from_bb = 1ULL << from;
  u64 to_bb = 1ULL << to;

  int pc = 0;
  for (int i = 0; i < 6; ++i)
    if (pos.bb[i] & from_bb) {
      pc = i;
      break;
    }

  pos.bb[pc] &= ~from_bb;
  pos.colors[us] &= ~from_bb;

  u64 cap_bb = to_bb & pos.occ;
  if (cap_bb) {
    for (int i = 0; i < 6; ++i)
      pos.bb[i] &= ~to_bb;
    pos.colors[them] &= ~to_bb;
    pos.halfmove = 0;
  } else {
    pos.halfmove++;
  }

  if (pc == 0)
    pos.halfmove = 0;

  if (f == 1) {
    u64 ep_cap = us == 0 ? (to_bb >> 8) : (to_bb << 8);
    pos.bb[0] &= ~ep_cap;
    pos.colors[them] &= ~ep_cap;
    pos.halfmove = 0;
  }

  if (f == 2) {
    if (to == 6) {
      pos.bb[3] &= ~0x80ULL;
      pos.bb[3] |= 0x20ULL;
      pos.colors[us] &= ~0x80ULL;
      pos.colors[us] |= 0x20ULL;
    }
    if (to == 2) {
      pos.bb[3] &= ~0x01ULL;
      pos.bb[3] |= 0x08ULL;
      pos.colors[us] &= ~0x01ULL;
      pos.colors[us] |= 0x08ULL;
    }
    if (to == 62) {
      pos.bb[3] &= ~0x8000000000000000ULL;
      pos.bb[3] |= 0x2000000000000000ULL;
      pos.colors[us] &= ~0x8000000000000000ULL;
      pos.colors[us] |= 0x2000000000000000ULL;
    }
    if (to == 58) {
      pos.bb[3] &= ~0x0100000000000000ULL;
      pos.bb[3] |= 0x0800000000000000ULL;
      pos.colors[us] &= ~0x0100000000000000ULL;
      pos.colors[us] |= 0x0800000000000000ULL;
    }
  }

  int place_pc = pc;
  if (f >= 3)
    place_pc = 6 - f;

  pos.bb[place_pc] |= to_bb;
  pos.colors[us] |= to_bb;
  pos.occ = pos.colors[0] | pos.colors[1];

  pos.ep_square = 64;
  if (pc == 0 && std::abs(from - to) == 16)
    pos.ep_square = (from + to) / 2;

  if (pc == 5) {
    if (us == 0)
      pos.castling &= ~3;
    else
      pos.castling &= ~12;
  }
  if (pc == 3) {
    if (from == 0)
      pos.castling &= ~2;
    if (from == 7)
      pos.castling &= ~1;
    if (from == 56)
      pos.castling &= ~8;
    if (from == 63)
      pos.castling &= ~4;
  }
  if (to == 0)
    pos.castling &= ~2;
  if (to == 7)
    pos.castling &= ~1;
  if (to == 56)
    pos.castling &= ~8;
  if (to == 63)
    pos.castling &= ~4;

  pos.white_to_move = !pos.white_to_move;
  int king_sq =
      __builtin_ctzll(pos.bb[5] & pos.colors[pos.white_to_move ? 0 : 1]);
  state.in_check = pos.is_attacked(king_sq, !pos.white_to_move);
}

// ============= PST & EVALUATION =============
constexpr std::array<int, 64> PST_PAWN = {
    0,  0,  0,  0,   0,   0,  0,  0,  50, 50, 50,  50, 50, 50,  50, 50,
    10, 10, 20, 30,  30,  20, 10, 10, 5,  5,  10,  25, 25, 10,  5,  5,
    0,  0,  0,  20,  20,  0,  0,  0,  5,  -5, -10, 0,  0,  -10, -5, 5,
    5,  10, 10, -20, -20, 10, 10, 5,  0,  0,  0,   0,  0,  0,   0,  0};
constexpr std::array<int, 64> PST_KNIGHT = {
    -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   0,   0,
    0,   -20, -40, -30, 0,   10,  15,  15,  10,  0,   -30, -30, 5,
    15,  20,  20,  15,  5,   -30, -30, 0,   15,  20,  20,  15,  0,
    -30, -30, 5,   10,  15,  15,  10,  5,   -30, -40, -20, 0,   5,
    5,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50};
constexpr std::array<int, 64> PST_BISHOP = {
    -20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,
    0,   0,   -10, -10, 0,   5,   10,  10,  5,   0,   -10, -10, 5,
    5,   10,  10,  5,   5,   -10, -10, 0,   10,  10,  10,  10,  0,
    -10, -10, 10,  10,  10,  10,  10,  10,  -10, -10, 5,   0,   0,
    0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20};
constexpr std::array<int, 64> PST_ROOK = {
    0,  0, 0, 0, 0, 0, 0, 0,  5,  10, 10, 10, 10, 10, 10, 5,
    -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
    -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
    -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5};
constexpr std::array<int, 64> PST_QUEEN = {
    -20, -10, -10, -5, -5, -10, -10, -20, -10, 0,   0,   0,  0,  0,   0,   -10,
    -10, 0,   5,   5,  5,  5,   0,   -10, -5,  0,   5,   5,  5,  5,   0,   -5,
    0,   0,   5,   5,  5,  5,   0,   -5,  -10, 5,   5,   5,  5,  5,   0,   -10,
    -10, 0,   5,   0,  0,  0,   0,   -10, -20, -10, -10, -5, -5, -10, -10, -20};
constexpr std::array<int, 64> PST_KING_MG = {
    -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50,
    -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40,
    -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30,
    -20, -10, -20, -20, -20, -20, -20, -20, -10, 20,  20,  0,   0,
    0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20};
constexpr std::array<int, 64> PST_KING_EG = {
    -50, -40, -30, -20, -20, -30, -40, -50, -30, -20, -10, 0,   0,
    -10, -20, -30, -30, -10, 20,  30,  30,  20,  -10, -30, -30, -10,
    30,  40,  40,  30,  -10, -30, -30, -10, 30,  40,  40,  30,  -10,
    -30, -30, -10, 20,  30,  30,  20,  -10, -30, -30, -30, 0,   0,
    0,   0,   -30, -30, -50, -30, -30, -30, -30, -30, -30, -50};

inline bool is_endgame(const Position &pos) {
  int w_q = __builtin_popcountll(pos.bb[4] & pos.colors[0]);
  int b_q = __builtin_popcountll(pos.bb[4] & pos.colors[1]);
  int w_r = __builtin_popcountll(pos.bb[3] & pos.colors[0]);
  int b_r = __builtin_popcountll(pos.bb[3] & pos.colors[1]);
  int w_m = __builtin_popcountll((pos.bb[1] | pos.bb[2]) & pos.colors[0]);
  int b_m = __builtin_popcountll((pos.bb[1] | pos.bb[2]) & pos.colors[1]);
  if (w_q == 0 && b_q == 0)
    return true;
  if (w_q == 0 && w_m <= 1 && w_r == 0)
    return true;
  if (b_q == 0 && b_m <= 1 && b_r == 0)
    return true;
  return false;
}

inline int evaluate_positional(const Position &pos, bool white_pov) {
  int score = 0;
  bool endgame = is_endgame(pos);
  for (int pc = 0; pc < 6; ++pc) {
    u64 w_bb = pos.bb[pc] & pos.colors[0];
    while (w_bb) {
      int sq = __builtin_ctzll(w_bb);
      int pst_idx = 63 - sq;
      switch (pc) {
      case 0:
        score += 100 + PST_PAWN[pst_idx];
        break;
      case 1:
        score += 320 + PST_KNIGHT[pst_idx];
        break;
      case 2:
        score += 330 + PST_BISHOP[pst_idx];
        break;
      case 3:
        score += 500 + PST_ROOK[pst_idx];
        break;
      case 4:
        score += 900 + PST_QUEEN[pst_idx];
        break;
      case 5:
        score +=
            20000 + (endgame ? PST_KING_EG[pst_idx] : PST_KING_MG[pst_idx]);
        break;
      }
      w_bb &= w_bb - 1;
    }
    u64 b_bb = pos.bb[pc] & pos.colors[1];
    while (b_bb) {
      int sq = __builtin_ctzll(b_bb);
      int pst_idx = sq;
      switch (pc) {
      case 0:
        score -= 100 + PST_PAWN[pst_idx];
        break;
      case 1:
        score -= 320 + PST_KNIGHT[pst_idx];
        break;
      case 2:
        score -= 330 + PST_BISHOP[pst_idx];
        break;
      case 3:
        score -= 500 + PST_ROOK[pst_idx];
        break;
      case 4:
        score -= 900 + PST_QUEEN[pst_idx];
        break;
      case 5:
        score -=
            20000 + (endgame ? PST_KING_EG[pst_idx] : PST_KING_MG[pst_idx]);
        break;
      }
      b_bb &= b_bb - 1;
    }
  }
  return white_pov ? score : -score;
}

// ============= ZOBRIST HASHING & TT =============
uint64_t zobrist_table[64][6][2];
uint64_t zobrist_white_to_move;
uint64_t zobrist_castling[16];
uint64_t zobrist_ep[65];

void init_zobrist() {
  std::mt19937_64 rng(0xDEADBEEF);
  for (int i = 0; i < 64; ++i)
    for (int p = 0; p < 6; ++p)
      for (int c = 0; c < 2; ++c)
        zobrist_table[i][p][c] = rng();
  zobrist_white_to_move = rng();
  for (int i = 0; i < 16; ++i)
    zobrist_castling[i] = rng();
  for (int i = 0; i < 65; ++i)
    zobrist_ep[i] = rng();
}

uint64_t hash_board(const Position &pos) {
  uint64_t h = 0;
  for (int pc = 0; pc < 6; ++pc) {
    u64 w_bb = pos.bb[pc] & pos.colors[0];
    while (w_bb) {
      int sq = __builtin_ctzll(w_bb);
      h ^= zobrist_table[sq][pc][0];
      w_bb &= w_bb - 1;
    }
    u64 b_bb = pos.bb[pc] & pos.colors[1];
    while (b_bb) {
      int sq = __builtin_ctzll(b_bb);
      h ^= zobrist_table[sq][pc][1];
      b_bb &= b_bb - 1;
    }
  }
  if (pos.white_to_move)
    h ^= zobrist_white_to_move;
  h ^= zobrist_castling[pos.castling];
  h ^= zobrist_ep[pos.ep_square];
  return h;
}

enum TTFlag { EXACT, LOWERBOUND, UPPERBOUND };
struct TTEntry {
  uint64_t hash = 0;
  int depth = -1;
  int score = 0;
  TTFlag flag = EXACT;
  Move best_move = 0;
};
const int TT_SIZE = 1 << 20;
std::vector<TTEntry> tt(TT_SIZE);

void tt_store(uint64_t hash, int depth, int score, TTFlag flag, Move move) {
  int idx = hash % TT_SIZE;
  if (tt[idx].depth <= depth || tt[idx].hash == 0)
    tt[idx] = {hash, depth, score, flag, move};
}

bool tt_probe(uint64_t hash, int depth, int alpha, int beta, int &score,
              Move &move) {
  int idx = hash % TT_SIZE;
  if (tt[idx].hash == hash) {
    move = tt[idx].best_move;
    if (tt[idx].depth >= depth) {
      if (tt[idx].flag == EXACT) {
        score = tt[idx].score;
        return true;
      }
      if (tt[idx].flag == LOWERBOUND && tt[idx].score >= beta) {
        score = tt[idx].score;
        return true;
      }
      if (tt[idx].flag == UPPERBOUND && tt[idx].score <= alpha) {
        score = tt[idx].score;
        return true;
      }
    }
  }
  return false;
}

// ============= SEARCH =============
inline int quiescence(Position &pos, int alpha, int beta, bool maximizing) {
  int stand_pat = evaluate_positional(pos, pos.white_to_move);
  if (maximizing) {
    if (stand_pat >= beta)
      return beta;
    if (stand_pat > alpha)
      alpha = stand_pat;
  } else {
    if (stand_pat <= alpha)
      return alpha;
    if (stand_pat < beta)
      beta = stand_pat;
  }

  MoveList list;
  generate_pseudo_legal_moves(pos, list);

  Move legal_moves[256];
  int legal_count = 0;
  for (int i = 0; i < list.count; ++i) {
    Move m = list.moves[i];
    int to = to_sq(m);
    int f = flags(m);
    bool is_cap = (pos.occ & (1ULL << to)) || (f == 1);
    bool is_promo = (f >= 3);
    if (!is_cap && !is_promo)
      continue;
    if (is_legal(pos, m))
      legal_moves[legal_count++] = m;
  }

  for (int i = 0; i < legal_count; ++i) {
    int to = to_sq(legal_moves[i]);
    int score = 0;
    if (pos.occ & (1ULL << to)) {
      int victim = 0;
      for (int p = 0; p < 6; ++p)
        if (pos.bb[p] & (1ULL << to)) {
          victim = p;
          break;
        }
      score = 100000 + victim * 10;
    }
    add_move_score(legal_moves[i], score);
  }
  std::sort(legal_moves, legal_moves + legal_count,
            [](Move a, Move b) { return move_score(a) > move_score(b); });

  for (int i = 0; i < legal_count; ++i) {
    Move m = legal_moves[i];
    Position next_pos = pos;
    GameState dummy_state;
    apply_move(next_pos, m, dummy_state);
    int eval = quiescence(next_pos, alpha, beta, !maximizing);
    if (maximizing) {
      if (eval > alpha)
        alpha = eval;
      if (alpha >= beta)
        return beta;
    } else {
      if (eval < beta)
        beta = eval;
      if (alpha >= beta)
        return alpha;
    }
  }
  return maximizing ? alpha : beta;
}

inline int minimax(Position &pos, int depth, int alpha, int beta,
                   bool maximizing) {
  uint64_t hash = hash_board(pos);
  Move tt_move = 0;
  int tt_score = 0;
  bool tt_hit = tt_probe(hash, depth, alpha, beta, tt_score, tt_move);
  if (tt_hit)
    return tt_score;

  if (depth == 0)
    return quiescence(pos, alpha, beta, maximizing);

  MoveList list;
  generate_pseudo_legal_moves(pos, list);

  Move legal_moves[256];
  int legal_count = 0;
  for (int i = 0; i < list.count; ++i) {
    if (is_legal(pos, list.moves[i]))
      legal_moves[legal_count++] = list.moves[i];
  }

  for (int i = 0; i < legal_count; ++i) {
    Move m = legal_moves[i];
    if (m == tt_move) {
      add_move_score(legal_moves[i], 2000000);
      continue;
    }
    int to = to_sq(m);
    int score = 0;
    if (pos.occ & (1ULL << to)) {
      int victim = 0, attacker = 0;
      for (int p = 0; p < 6; ++p)
        if (pos.bb[p] & (1ULL << to)) {
          victim = p;
          break;
        }
      u64 from_bb = 1ULL << from_sq(m);
      for (int p = 0; p < 6; ++p)
        if (pos.bb[p] & from_bb) {
          attacker = p;
          break;
        }
      score = 100000 + victim * 10 - attacker;
    }
    add_move_score(legal_moves[i], score);
  }
  std::sort(legal_moves, legal_moves + legal_count,
            [](Move a, Move b) { return move_score(a) > move_score(b); });

  if (legal_count == 0) {
    int king_sq =
        __builtin_ctzll(pos.bb[5] & pos.colors[pos.white_to_move ? 0 : 1]);
    if (pos.is_attacked(king_sq, !pos.white_to_move))
      return maximizing ? -50000 + depth : 50000 - depth;
    return 0;
  }

  int origAlpha = alpha;
  Move best_move = legal_moves[0];

  if (maximizing) {
    int maxEval = -1000000;
    for (int i = 0; i < legal_count; ++i) {
      Move m = legal_moves[i];
      Position next_pos = pos;
      GameState dummy_state;
      apply_move(next_pos, m, dummy_state);
      int eval = minimax(next_pos, depth - 1, alpha, beta, false);
      if (eval > maxEval) {
        maxEval = eval;
        best_move = m;
      }
      alpha = std::max(alpha, eval);
      if (beta <= alpha)
        break;
    }
    TTFlag flag = (maxEval <= origAlpha) ? UPPERBOUND
                  : (maxEval >= beta)    ? LOWERBOUND
                                         : EXACT;
    tt_store(hash, depth, maxEval, flag, best_move);
    return maxEval;
  } else {
    int minEval = 1000000;
    for (int i = 0; i < legal_count; ++i) {
      Move m = legal_moves[i];
      Position next_pos = pos;
      GameState dummy_state;
      apply_move(next_pos, m, dummy_state);
      int eval = minimax(next_pos, depth - 1, alpha, beta, true);
      if (eval < minEval) {
        minEval = eval;
        best_move = m;
      }
      beta = std::min(beta, eval);
      if (beta <= alpha)
        break;
    }
    TTFlag flag = (minEval <= origAlpha) ? UPPERBOUND
                  : (minEval >= beta)    ? LOWERBOUND
                                         : EXACT;
    tt_store(hash, depth, minEval, flag, best_move);
    return minEval;
  }
}

// ============= UI & EVENT HANDLING =============
sf::Font fontInBuildDirectory{sf::Font("src/PlaywriteGBSGuides-Regular.ttf")};
sf::Text defaultTextToAvoidError{fontInBuildDirectory, "something", 30};

struct Button {
  sf::RectangleShape shape;
  sf::Text text{defaultTextToAvoidError};
  bool hovered = false;
  Button(const sf::Font &font, const std::string &label, sf::Vector2f pos,
         sf::Vector2f size) {
    shape.setSize(size);
    shape.setPosition(pos);
    shape.setFillColor(BUTTON_COLOR);
    text.setFont(font);
    text.setString(label);
    text.setCharacterSize(28);
    text.setFillColor(BUTTON_TEXT_COLOR);
    sf::FloatRect b = text.getLocalBounds();
    text.setOrigin(
        {b.position.x + b.size.x / 2.0f, b.position.y + b.size.y / 2.0f});
    text.setPosition({pos.x + size.x / 2.0f, pos.y + size.y / 2.0f});
  }
  bool contains(sf::Vector2f p) const {
    return shape.getGlobalBounds().contains(p);
  }
  void setHovered(bool h) {
    hovered = h;
    shape.setFillColor(h ? BUTTON_HOVER_COLOR : BUTTON_COLOR);
  }
  void draw(sf::RenderWindow &w) const {
    w.draw(shape);
    w.draw(text);
  }
};

inline void draw_board(const Position &pos, sf::RenderWindow &window,
                       sf::RectangleShape &sq, sf::Sprite &ps,
                       const GameState &state, sf::RectangleShape &hl) {
  constexpr float SQ = 64.0f;
  for (std::uint8_t i = 0; i < 64; ++i) {
    int r = i / 8, f = i % 8;
    sf::Color c = ((f + r) % 2 == 0) ? DUSTY_GRAPE : LIGHT_YELLOW;
    if (state.selected_square == i)
      c = SELECTION_COLOR;
    else if (state.last_from == i || state.last_to == i)
      c = HIGHLIGHT_COLOR;
    else if (state.in_check &&
             get_piece_at(pos, i) ==
                 (pos.white_to_move ? Piece::WHITE_KING : Piece::BLACK_KING))
      c = CHECK_COLOR;
    sq.setFillColor(c);
    sq.setPosition(board_index_to_pixel(i, state.player_is_white));
    window.draw(sq);
    Piece p = get_piece_at(pos, i);
    if (p != Piece::NONE) {
      ps.setTextureRect(get_texture_rect(p));
      ps.setPosition(board_index_to_pixel(i, state.player_is_white));
      window.draw(ps);
    }
  }
  if (state.selected_square) {
    std::uint8_t from = state.selected_square.value();
    MoveList list;
    generate_pseudo_legal_moves(pos, list);
    for (int i = 0; i < list.count; ++i) {
      Move m = list.moves[i];
      if (from_sq(m) == from && is_legal(pos, m)) {
        std::uint8_t t = to_sq(m);
        sf::Vector2f p = board_index_to_pixel(t, state.player_is_white);
        if (get_piece_at(pos, t) != Piece::NONE) {
          hl.setPosition({p.x + 4, p.y + 4});
          hl.setSize({SQ - 8, SQ - 8});
          hl.setFillColor(sf::Color::Transparent);
          hl.setOutlineColor(VALID_MOVE_COLOR);
          hl.setOutlineThickness(4);
        } else {
          hl.setPosition({p.x + SQ / 2 - 8, p.y + SQ / 2 - 8});
          hl.setSize({16, 16});
          hl.setFillColor(VALID_MOVE_COLOR);
          hl.setOutlineThickness(0);
        }
        window.draw(hl);
      }
    }
  }
}

inline Piece show_promotion_dialog(sf::RenderWindow &window,
                                   const sf::Font &font, bool is_white,
                                   const Position &pos, const GameState &state,
                                   sf::RectangleShape &sq, sf::Sprite &ps,
                                   sf::RectangleShape &hl) {
  std::vector<Button> buttons;
  std::vector<Piece> choices;
  std::vector<std::string> labels = {"Q", "R", "B", "N"};
  if (is_white)
    choices = {Piece::WHITE_QUEEN, Piece::WHITE_ROOK, Piece::WHITE_BISHOP,
               Piece::WHITE_KNIGHT};
  else
    choices = {Piece::BLACK_QUEEN, Piece::BLACK_ROOK, Piece::BLACK_BISHOP,
               Piece::BLACK_KNIGHT};
  float btn_size = 50, gap = 10, start_x = 140, y = 275;
  for (int i = 0; i < 4; ++i)
    buttons.emplace_back(Button{font,
                                labels[i],
                                {start_x + i * (btn_size + gap), y},
                                {btn_size, btn_size}});
  sf::RectangleShape overlay{{576, 576}};
  overlay.setFillColor(sf::Color(0, 0, 0, 150));
  sf::RectangleShape dialog{{300, 100}};
  dialog.setFillColor(BUTTON_COLOR);
  dialog.setPosition({138, 238});
  sf::Text prompt(font, "Choose promotion:", 20);
  prompt.setFillColor(TITLE_COLOR);
  prompt.setPosition({160, 245});
  while (window.isOpen()) {
    while (const auto ev = window.pollEvent()) {
      if (ev->is<sf::Event::Closed>()) {
        window.close();
        return Piece::NONE;
      }
      if (const auto *mm = ev->getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mp(static_cast<float>(mm->position.x),
                        static_cast<float>(mm->position.y));
        for (auto &b : buttons)
          b.setHovered(b.contains(mp));
      }
      if (const auto *mc = ev->getIf<sf::Event::MouseButtonPressed>()) {
        if (mc->button == sf::Mouse::Button::Left) {
          sf::Vector2f mp(static_cast<float>(mc->position.x),
                          static_cast<float>(mc->position.y));
          for (size_t i = 0; i < buttons.size(); ++i)
            if (buttons[i].contains(mp))
              return choices[i];
        }
      }
    }
    window.clear(OCEAN_MIST);
    draw_board(pos, window, sq, ps, state, hl);
    window.draw(overlay);
    window.draw(dialog);
    window.draw(prompt);
    for (auto &b : buttons)
      b.draw(window);
    window.display();
  }
  return Piece::NONE;
}

enum class AppState { SIDE_SELECTION, PLAYING };
inline bool run_side_selection(sf::RenderWindow &window, GameState &state,
                               const sf::Font &font) {
  AppState app = AppState::SIDE_SELECTION;
  sf::Text title(font, "Choose Your Side", 42);
  title.setFillColor(TITLE_COLOR);
  sf::FloatRect tb = title.getLocalBounds();
  title.setOrigin(
      {tb.position.x + tb.size.x / 2, tb.position.y + tb.size.y / 2});
  title.setPosition({288, 150});
  sf::Text sub(font, "White pieces move first", 18);
  sub.setFillColor(sf::Color(200, 200, 200));
  sf::FloatRect sb = sub.getLocalBounds();
  sub.setOrigin({sb.position.x + sb.size.x / 2, sb.position.y + sb.size.y / 2});
  sub.setPosition({288, 200});
  Button wb(font, "Play as White", {188, 260}, {200, 60});
  Button bb(font, "Play as Black", {188, 350}, {200, 60});
  while (window.isOpen() && app == AppState::SIDE_SELECTION) {
    while (const auto ev = window.pollEvent()) {
      if (ev->is<sf::Event::Closed>()) {
        window.close();
        return false;
      }
      if (const auto *mm = ev->getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mp(static_cast<float>(mm->position.x),
                        static_cast<float>(mm->position.y));
        wb.setHovered(wb.contains(mp));
        bb.setHovered(bb.contains(mp));
      }
      if (const auto *mc = ev->getIf<sf::Event::MouseButtonPressed>()) {
        if (mc->button == sf::Mouse::Button::Left) {
          sf::Vector2f mp(static_cast<float>(mc->position.x),
                          static_cast<float>(mc->position.y));
          if (wb.contains(mp)) {
            state.player_is_white = true;
            app = AppState::PLAYING;
          } else if (bb.contains(mp)) {
            state.player_is_white = false;
            app = AppState::PLAYING;
          }
        }
      }
    }
    window.clear(DUSTY_GRAPE);
    window.draw(title);
    window.draw(sub);
    wb.draw(window);
    bb.draw(window);
    window.display();
  }
  return window.isOpen();
}

inline bool is_valid_move_ui_highlight(const Position &pos, int from, int to) {
  MoveList list;
  generate_pseudo_legal_moves(pos, list);
  for (int i = 0; i < list.count; ++i) {
    Move m = list.moves[i];
    if (from_sq(m) == from && to_sq(m) == to && is_legal(pos, m))
      return true;
  }
  return false;
}

inline void handle_events(sf::RenderWindow &window, Position &pos,
                          GameState &state,
                          std::optional<Move> &pending_promo_move) {
  while (const auto ev = window.pollEvent()) {
    if (ev->is<sf::Event::Closed>())
      window.close();
    else if (const auto *mc = ev->getIf<sf::Event::MouseButtonPressed>()) {
      if (mc->button == sf::Mouse::Button::Left) {
        if (state.game_over) {
          state.restart_requested = true;
          continue;
        }
        if (pos.white_to_move != state.player_is_white)
          continue;
        auto idx = pixel_to_board_index(mc->position.x, mc->position.y,
                                        state.player_is_white);
        if (!idx) {
          state.selected_square = std::nullopt;
          continue;
        }
        std::uint8_t t = idx.value();
        if (!state.selected_square) {
          Piece p = get_piece_at(pos, t);
          if (p != Piece::NONE && is_white_piece(p) == pos.white_to_move)
            state.selected_square = t;
          continue;
        }
        std::uint8_t f = state.selected_square.value();
        if (f == t) {
          state.selected_square = std::nullopt;
          continue;
        }
        if (is_valid_move_ui_highlight(pos, f, t)) {
          bool is_promo = false;
          MoveList list;
          generate_pseudo_legal_moves(pos, list);
          for (int i = 0; i < list.count; ++i) {
            Move m = list.moves[i];
            if (from_sq(m) == f && to_sq(m) == t && flags(m) >= 3 &&
                is_legal(pos, m)) {
              is_promo = true;
              break;
            }
          }
          if (is_promo)
            pending_promo_move = make_move(f, t);
          else {
            for (int i = 0; i < list.count; ++i) {
              Move m = list.moves[i];
              if (from_sq(m) == f && to_sq(m) == t && is_legal(pos, m)) {
                apply_move(pos, m, state);
                break;
              }
            }
          }
          state.selected_square = std::nullopt;
        } else {
          Piece p = get_piece_at(pos, t);
          if (p != Piece::NONE && is_white_piece(p) == pos.white_to_move)
            state.selected_square = t;
          else
            state.selected_square = std::nullopt;
        }
      }
    }
  }
}

inline void check_game_over(const Position &pos, GameState &state) {
  MoveList list;
  generate_pseudo_legal_moves(pos, list);
  bool has_legal = false;
  for (int i = 0; i < list.count; ++i)
    if (is_legal(pos, list.moves[i])) {
      has_legal = true;
      break;
    }
  if (!has_legal) {
    state.game_over = true;
    int king_sq =
        __builtin_ctzll(pos.bb[5] & pos.colors[pos.white_to_move ? 0 : 1]);
    if (pos.is_attacked(king_sq, !pos.white_to_move))
      state.result_message = pos.white_to_move ? "Black wins! (Checkmate)"
                                               : "White wins! (Checkmate)";
    else
      state.result_message = "Draw! (Stalemate)";
    return;
  }
  if (pos.halfmove >= 100) {
    state.game_over = true;
    state.result_message = "Draw! (50-move rule)";
    return;
  }
  int w_pieces = __builtin_popcountll(pos.colors[0]);
  int b_pieces = __builtin_popcountll(pos.colors[1]);
  int w_minor = __builtin_popcountll((pos.bb[1] | pos.bb[2]) & pos.colors[0]);
  int b_minor = __builtin_popcountll((pos.bb[1] | pos.bb[2]) & pos.colors[1]);
  if (w_pieces == 1 && b_pieces == 1) {
    state.game_over = true;
    state.result_message = "Draw! (Insufficient material)";
  } else if (w_pieces == 2 && b_pieces == 1 && w_minor == 1) {
    state.game_over = true;
    state.result_message = "Draw! (Insufficient material)";
  } else if (w_pieces == 1 && b_pieces == 2 && b_minor == 1) {
    state.game_over = true;
    state.result_message = "Draw! (Insufficient material)";
  }
}

int main() {
  init_bitboards();
  init_zobrist();
  sf::RenderWindow window{sf::VideoMode{{576, 576}}, "Chess vs Bot",
                          sf::Style::Close};
  sf::Font font;
  bool loaded = false;
  const char *paths[] = {
      "src/PlaywriteGBSGuides-Regular.ttf",
      "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
      "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
      "/usr/share/fonts/TTF/DejaVuSans.ttf",
      "/System/Library/Fonts/Helvetica.ttc",
      "C:/Windows/Fonts/arial.ttf",
      nullptr};
  for (int i = 0; paths[i] && !loaded; ++i)
    loaded = font.openFromFile(paths[i]);
  if (!loaded)
    std::cerr << "Warning: Could not load font.\n";
  GameState state{};
  if (loaded && !run_side_selection(window, state, font))
    return 0;
  else if (!loaded)
    state.player_is_white = true;
  Position pos = make_initial_position();
  sf::RectangleShape sq{{64, 64}}, hl{{64, 64}};
  hl.setFillColor(sf::Color::Transparent);
  sf::Texture tex;
  if (!tex.loadFromMemory(chess_pieces, chess_pieces_size)) {
    std::cerr << "Texture load failed!\n";
    return 1;
  }
  sf::Sprite ps{tex};

  std::thread ai_thread;
  std::mutex ai_mtx;
  Move ai_best_move = 0;
  bool ai_ready = false;
  std::atomic<int> search_gen{0};

  auto make_ai_worker = [&](Position p, int gen) {
    return [&, p, gen]() mutable {
      Move current_best = 0;
      bool found = false;
      for (int depth = 1; depth <= 6; ++depth) {
        if (gen != search_gen.load())
          return;
        MoveList list;
        generate_pseudo_legal_moves(p, list);
        Move legal_moves[256];
        int legal_count = 0;
        for (int i = 0; i < list.count; ++i)
          if (is_legal(p, list.moves[i]))
            legal_moves[legal_count++] = list.moves[i];
        if (legal_count == 0)
          return;
        Move best = legal_moves[0];
        int best_eval = p.white_to_move ? -1000000 : 1000000;
        for (int i = 0; i < legal_count; ++i) {
          if (gen != search_gen.load())
            return;
          Move m = legal_moves[i];
          Position nb = p;
          GameState ns;
          apply_move(nb, m, ns);
          int eval =
              minimax(nb, depth - 1, -1000000, 1000000, !p.white_to_move);
          if ((p.white_to_move && eval > best_eval) ||
              (!p.white_to_move && eval < best_eval)) {
            best_eval = eval;
            best = m;
          }
        }
        current_best = best;
        found = true;
      }
      if (found) {
        std::lock_guard<std::mutex> lock(ai_mtx);
        ai_best_move = current_best;
        ai_ready = true;
      }
    };
  };

  while (window.isOpen()) {
    std::optional<Move> pending_promo_move;
    handle_events(window, pos, state, pending_promo_move);

    if (state.restart_requested) {
      search_gen.fetch_add(1);
      if (ai_thread.joinable())
        ai_thread.join();
      {
        std::lock_guard<std::mutex> lock(ai_mtx);
        ai_ready = false;
      }
      bool wp = state.player_is_white;
      state = GameState{};
      state.player_is_white = wp;
      pos = make_initial_position();
      continue;
    }

    if (pending_promo_move) {
      Move m_base = pending_promo_move.value();
      bool is_white = pos.white_to_move;
      Piece promo =
          show_promotion_dialog(window, font, is_white, pos, state, sq, ps, hl);
      if (promo != Piece::NONE) {
        int req_f = 6;
        if (promo == Piece::WHITE_ROOK || promo == Piece::BLACK_ROOK)
          req_f = 5;
        else if (promo == Piece::WHITE_BISHOP || promo == Piece::BLACK_BISHOP)
          req_f = 4;
        else if (promo == Piece::WHITE_KNIGHT || promo == Piece::BLACK_KNIGHT)
          req_f = 3;
        Move m = make_move(from_sq(m_base), to_sq(m_base), req_f);
        apply_move(pos, m, state);
      }
    }

    if (!state.game_over && pos.white_to_move != state.player_is_white) {
      if (ai_thread.joinable() && ai_ready) {
        ai_thread.join();
        Move move_to_play;
        {
          std::lock_guard<std::mutex> lock(ai_mtx);
          move_to_play = ai_best_move;
          ai_ready = false;
        }
        apply_move(pos, move_to_play, state);
        check_game_over(pos, state);
      } else if (ai_thread.joinable()) {
        std::lock_guard<std::mutex> lock(ai_mtx);
        if (ai_ready) {
        }
      }
      if (!state.game_over && pos.white_to_move != state.player_is_white &&
          !ai_thread.joinable()) {
        int gen = search_gen.fetch_add(1) + 1;
        ai_thread = std::thread(make_ai_worker(pos, gen));
      }
    } else {
      if (ai_thread.joinable()) {
        search_gen.fetch_add(1);
        ai_thread.join();
        std::lock_guard<std::mutex> lock(ai_mtx);
        ai_ready = false;
      }
    }

    window.clear(OCEAN_MIST);
    draw_board(pos, window, sq, ps, state, hl);
    if (state.game_over) {
      sf::RectangleShape ov{{576, 576}};
      ov.setFillColor(sf::Color(0, 0, 0, 150));
      window.draw(ov);
      sf::Text gt(font, state.result_message, 36);
      gt.setFillColor(sf::Color::White);
      sf::FloatRect gb = gt.getLocalBounds();
      gt.setOrigin(
          {gb.position.x + gb.size.x / 2, gb.position.y + gb.size.y / 2});
      gt.setPosition({288, 260});
      window.draw(gt);
      sf::Text rt(font, "Click anywhere to restart", 20);
      rt.setFillColor(sf::Color(200, 200, 200));
      sf::FloatRect rb = rt.getLocalBounds();
      rt.setOrigin(
          {rb.position.x + rb.size.x / 2, rb.position.y + rb.size.y / 2});
      rt.setPosition({288, 320});
      window.draw(rt);
    }
    window.display();
  }

  search_gen.fetch_add(1);
  if (ai_thread.joinable())
    ai_thread.join();
  return 0;
}