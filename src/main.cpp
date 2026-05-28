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
#include <cmath>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
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
enum class SquareColor : std::uint8_t { DARK, LIGHT };

// ============= BOARD REPRESENTATION =============
struct SquareInfo {
  Piece piece = Piece::NONE;
  SquareColor color = SquareColor::DARK;
};
using Board = std::array<SquareInfo, 64>;

// ============= GAME STATE =============
struct GameState {
  std::optional<std::uint8_t> selected_square;
  std::optional<std::uint8_t> last_from;
  std::optional<std::uint8_t> last_to;
  bool white_to_move = true;
  bool in_check = false;
  std::uint8_t white_king_pos = 4;
  std::uint8_t black_king_pos = 60;
  bool white_kingside_castle = true;
  bool white_queenside_castle = true;
  bool black_kingside_castle = true;
  bool black_queenside_castle = true;
  std::optional<std::uint8_t> en_passant_target;
  std::uint8_t halfmove_clock = 0;
  std::uint16_t fullmove_number = 1;
  bool player_is_white = false;
  bool game_over = false;
  std::string result_message = "";
};

// ============= BOARD INITIALIZATION =============
constexpr Board make_canonical_board() {
  Board board{};
  for (std::uint8_t idx = 0; idx < 64; ++idx) {
    std::uint8_t file = idx % 8;
    std::uint8_t rank = idx / 8;
    board[idx].color =
        ((file + rank) % 2 == 0) ? SquareColor::DARK : SquareColor::LIGHT;
  }
  board[0].piece = Piece::WHITE_ROOK;
  board[1].piece = Piece::WHITE_KNIGHT;
  board[2].piece = Piece::WHITE_BISHOP;
  board[3].piece = Piece::WHITE_QUEEN;
  board[4].piece = Piece::WHITE_KING;
  board[5].piece = Piece::WHITE_BISHOP;
  board[6].piece = Piece::WHITE_KNIGHT;
  board[7].piece = Piece::WHITE_ROOK;
  for (std::uint8_t f = 0; f < 8; ++f)
    board[8 + f].piece = Piece::WHITE_PAWN;
  for (std::uint8_t f = 0; f < 8; ++f)
    board[48 + f].piece = Piece::BLACK_PAWN;
  board[56].piece = Piece::BLACK_ROOK;
  board[57].piece = Piece::BLACK_KNIGHT;
  board[58].piece = Piece::BLACK_BISHOP;
  board[59].piece = Piece::BLACK_QUEEN;
  board[60].piece = Piece::BLACK_KING;
  board[61].piece = Piece::BLACK_BISHOP;
  board[62].piece = Piece::BLACK_KNIGHT;
  board[63].piece = Piece::BLACK_ROOK;
  return board;
}

// ============= PIECE UTILITY FUNCTIONS =============
inline bool is_white_piece(Piece p) noexcept {
  return p != Piece::NONE && static_cast<std::uint8_t>(p) < 6;
}
inline bool is_black_piece(Piece p) noexcept {
  return p != Piece::NONE && static_cast<std::uint8_t>(p) >= 6;
}
inline bool is_same_color(Piece a, Piece b) noexcept {
  if (a == Piece::NONE || b == Piece::NONE)
    return false;
  return is_white_piece(a) == is_white_piece(b);
}
inline Piece piece_type(Piece p) noexcept {
  return p == Piece::NONE
             ? Piece::NONE
             : static_cast<Piece>(static_cast<std::uint8_t>(p) % 6);
}

// ============= INDEX HELPERS =============
constexpr std::uint8_t get_file(std::uint8_t idx) noexcept { return idx % 8; }
constexpr std::uint8_t get_rank(std::uint8_t idx) noexcept { return idx / 8; }
constexpr std::uint8_t file_rank_to_index(std::uint8_t file,
                                          std::uint8_t rank) noexcept {
  return rank * 8 + file;
}

// ============= TEXTURE ATLAS LOOKUP =============
inline sf::IntRect get_texture_rect(Piece piece) noexcept {
  constexpr int PIECE_SIZE = 64;
  if (piece == Piece::NONE)
    return {{0, 0}, {0, 0}};
  std::uint8_t idx = static_cast<std::uint8_t>(piece);
  bool is_black = idx >= 6;
  return {{(idx % 6) * PIECE_SIZE, static_cast<int>(is_black) * PIECE_SIZE},
          {PIECE_SIZE, PIECE_SIZE}};
}

// ============= MOUSE & COORDINATE HELPERS =============
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
  return {static_cast<float>(get_file(idx) * SQ + MARGIN),
          static_cast<float>(
              board_rank_to_display_rank(get_rank(idx), player_is_white) * SQ +
              MARGIN)};
}

// ============================================================
// MOVE VALIDATION ENGINE
// ============================================================
inline bool is_path_clear(const Board &board, std::uint8_t from,
                          std::uint8_t to) {
  std::uint8_t ff = get_file(from), fr = get_rank(from);
  std::uint8_t tf = get_file(to), tr = get_rank(to);
  int fs = (tf > ff) ? 1 : (tf < ff) ? -1 : 0;
  int rs = (tr > fr) ? 1 : (tr < fr) ? -1 : 0;
  for (std::uint8_t cf = ff + fs, cr = fr + rs; cf != tf || cr != tr;
       cf += fs, cr += rs) {
    if (board[file_rank_to_index(cf, cr)].piece != Piece::NONE)
      return false;
  }
  return true;
}
inline bool is_square_attacked(const Board &board, std::uint8_t sq,
                               bool by_white) {
  std::uint8_t sf = get_file(sq), sr = get_rank(sq);
  int pd = by_white ? -1 : 1;
  std::uint8_t ar = sr + pd;
  if (ar < 8) {
    if (sf > 0 && board[file_rank_to_index(sf - 1, ar)].piece ==
                      (by_white ? Piece::WHITE_PAWN : Piece::BLACK_PAWN))
      return true;
    if (sf < 7 && board[file_rank_to_index(sf + 1, ar)].piece ==
                      (by_white ? Piece::WHITE_PAWN : Piece::BLACK_PAWN))
      return true;
  }
  constexpr int km[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                            {1, -2},  {1, 2},  {2, -1},  {2, 1}};
  for (auto &m : km) {
    int nf = sf + m[0], nr = sr + m[1];
    if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
      Piece p = board[file_rank_to_index(nf, nr)].piece;
      if (p != Piece::NONE && is_white_piece(p) == by_white &&
          piece_type(p) == Piece::WHITE_KNIGHT)
        return true;
    }
  }
  for (int df = -1; df <= 1; ++df)
    for (int dr = -1; dr <= 1; ++dr) {
      if (df == 0 && dr == 0)
        continue;
      int nf = sf + df, nr = sr + dr;
      if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
        Piece p = board[file_rank_to_index(nf, nr)].piece;
        if (p != Piece::NONE && is_white_piece(p) == by_white &&
            piece_type(p) == Piece::WHITE_KING)
          return true;
      }
    }
  constexpr int rd[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
  for (auto &d : rd) {
    for (int cf = sf + d[0], cr = sr + d[1];
         cf >= 0 && cf < 8 && cr >= 0 && cr < 8; cf += d[0], cr += d[1]) {
      Piece p = board[file_rank_to_index(cf, cr)].piece;
      if (p != Piece::NONE) {
        if (is_white_piece(p) == by_white &&
            (piece_type(p) == Piece::WHITE_ROOK ||
             piece_type(p) == Piece::WHITE_QUEEN))
          return true;
        break;
      }
    }
  }
  constexpr int bd[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
  for (auto &d : bd) {
    for (int cf = sf + d[0], cr = sr + d[1];
         cf >= 0 && cf < 8 && cr >= 0 && cr < 8; cf += d[0], cr += d[1]) {
      Piece p = board[file_rank_to_index(cf, cr)].piece;
      if (p != Piece::NONE) {
        if (is_white_piece(p) == by_white &&
            (piece_type(p) == Piece::WHITE_BISHOP ||
             piece_type(p) == Piece::WHITE_QUEEN))
          return true;
        break;
      }
    }
  }
  return false;
}
inline bool is_in_check(const Board &board, bool white_king) {
  std::uint8_t kp = white_king ? 4 : 60;
  for (std::uint8_t i = 0; i < 64; ++i) {
    if (board[i].piece ==
        (white_king ? Piece::WHITE_KING : Piece::BLACK_KING)) {
      kp = i;
      break;
    }
  }
  return is_square_attacked(board, kp, !white_king);
}

// Piece-Square Tables
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

inline int evaluate_positional(const Board &board, bool white_pov) {
  int score = 0;
  for (std::uint8_t i = 0; i < 64; ++i) {
    Piece p = board[i].piece;
    if (p == Piece::NONE)
      continue;
    int idx = white_pov ? i : (63 - i);
    switch (piece_type(p)) {
    case Piece::WHITE_PAWN:
      score += is_white_piece(p) ? (100 + PST_PAWN[idx])
                                 : -(100 + PST_PAWN[63 - idx]);
      break;
    case Piece::WHITE_KNIGHT:
      score += is_white_piece(p) ? (320 + PST_KNIGHT[idx])
                                 : -(320 + PST_KNIGHT[63 - idx]);
      break;
    case Piece::WHITE_BISHOP:
      score += is_white_piece(p) ? 330 : -330;
      break;
    case Piece::WHITE_ROOK:
      score += is_white_piece(p) ? 500 : -500;
      break;
    case Piece::WHITE_QUEEN:
      score += is_white_piece(p) ? 900 : -900;
      break;
    case Piece::WHITE_KING:
      score += is_white_piece(p) ? 20000 : -20000;
      break;
    default:
      break;
    }
  }
  return white_pov ? score : -score;
}

inline bool is_valid_pawn_move(const Board &board, std::uint8_t from,
                               std::uint8_t to, const GameState &state,
                               bool is_white) {
  std::uint8_t ff = get_file(from), fr = get_rank(from);
  std::uint8_t tf = get_file(to), tr = get_rank(to);
  int dir = is_white ? 1 : -1;
  int rd = static_cast<int>(tr) - static_cast<int>(fr);
  int fd = static_cast<int>(tf) - static_cast<int>(ff);
  if (fd == 0) {
    if (rd == dir)
      return board[to].piece == Piece::NONE;
    if (rd == 2 * dir) {
      std::uint8_t mid = file_rank_to_index(ff, fr + dir);
      return board[to].piece == Piece::NONE &&
             board[mid].piece == Piece::NONE &&
             ((is_white && fr == 1) || (!is_white && fr == 6));
    }
  }
  if (std::abs(fd) == 1 && rd == dir) {
    if (board[to].piece != Piece::NONE &&
        !is_same_color(board[from].piece, board[to].piece))
      return true;
    if (state.en_passant_target && to == state.en_passant_target.value())
      return true;
  }
  return false;
}
inline bool is_valid_knight_move(std::uint8_t from, std::uint8_t to) {
  int fd = std::abs(static_cast<int>(get_file(to)) -
                    static_cast<int>(get_file(from)));
  int rd = std::abs(static_cast<int>(get_rank(to)) -
                    static_cast<int>(get_rank(from)));
  return (fd == 2 && rd == 1) || (fd == 1 && rd == 2);
}
inline bool is_valid_bishop_move(const Board &board, std::uint8_t from,
                                 std::uint8_t to) {
  int fd = std::abs(static_cast<int>(get_file(to)) -
                    static_cast<int>(get_file(from)));
  int rd = std::abs(static_cast<int>(get_rank(to)) -
                    static_cast<int>(get_rank(from)));
  return fd != 0 && fd == rd && is_path_clear(board, from, to);
}
inline bool is_valid_rook_move(const Board &board, std::uint8_t from,
                               std::uint8_t to) {
  bool same_file = get_file(from) == get_file(to);
  bool same_rank = get_rank(from) == get_rank(to);
  return (same_file || same_rank) && is_path_clear(board, from, to);
}
inline bool is_valid_queen_move(const Board &board, std::uint8_t from,
                                std::uint8_t to) {
  return is_valid_bishop_move(board, from, to) ||
         is_valid_rook_move(board, from, to);
}
inline bool is_valid_king_move(std::uint8_t from, std::uint8_t to) {
  int fd = std::abs(static_cast<int>(get_file(to)) -
                    static_cast<int>(get_file(from)));
  int rd = std::abs(static_cast<int>(get_rank(to)) -
                    static_cast<int>(get_rank(from)));
  return fd <= 1 && rd <= 1 && (fd + rd > 0);
}
inline bool can_castle(const Board &board, const GameState &state,
                       bool is_white, bool kingside) {
  if (is_white) {
    if (kingside && !state.white_kingside_castle)
      return false;
    if (!kingside && !state.white_queenside_castle)
      return false;
  } else {
    if (kingside && !state.black_kingside_castle)
      return false;
    if (!kingside && !state.black_queenside_castle)
      return false;
  }
  std::uint8_t kr = is_white ? 0 : 7, kp = file_rank_to_index(4, kr);
  if (board[kp].piece != (is_white ? Piece::WHITE_KING : Piece::BLACK_KING))
    return false;
  if (is_in_check(board, is_white))
    return false;
  std::uint8_t rf = kingside ? 7 : 0, rp = file_rank_to_index(rf, kr);
  if (board[rp].piece != (is_white ? Piece::WHITE_ROOK : Piece::BLACK_ROOK))
    return false;
  int step = kingside ? 1 : -1;
  for (int f = 4 + step; f != rf; f += step)
    if (board[file_rank_to_index(f, kr)].piece != Piece::NONE)
      return false;
  for (int f = 4 + step; (kingside ? f <= 6 : f >= 2); f += step) {
    if (is_square_attacked(board, file_rank_to_index(f, kr), !is_white))
      return false;
    if (f == (kingside ? 6 : 2))
      break;
  }
  return true;
}
inline bool is_valid_move(const Board &board, std::uint8_t from,
                          std::uint8_t to, const GameState &state) {
  if (from == to || from >= 64 || to >= 64)
    return false;
  Piece mp = board[from].piece;
  if (mp == Piece::NONE || is_white_piece(mp) != state.white_to_move)
    return false;
  if (board[to].piece != Piece::NONE && is_same_color(mp, board[to].piece))
    return false;
  bool valid = false;
  switch (piece_type(mp)) {
  case Piece::WHITE_PAWN:
    valid = is_valid_pawn_move(board, from, to, state, is_white_piece(mp));
    break;
  case Piece::WHITE_KNIGHT:
    valid = is_valid_knight_move(from, to);
    break;
  case Piece::WHITE_BISHOP:
    valid = is_valid_bishop_move(board, from, to);
    break;
  case Piece::WHITE_ROOK:
    valid = is_valid_rook_move(board, from, to);
    break;
  case Piece::WHITE_QUEEN:
    valid = is_valid_queen_move(board, from, to);
    break;
  case Piece::WHITE_KING:
    valid = is_valid_king_move(from, to);
    if (!valid && get_rank(from) == get_rank(to) && get_file(from) == 4) {
      if (get_file(to) == 6)
        valid = can_castle(board, state, is_white_piece(mp), true);
      else if (get_file(to) == 2)
        valid = can_castle(board, state, is_white_piece(mp), false);
    }
    break;
  default:
    break;
  }
  if (!valid)
    return false;
  // Simulate move
  Board test = board;
  test[to].piece = test[from].piece;
  test[from].piece = Piece::NONE;
  if (piece_type(test[to].piece) == Piece::WHITE_PAWN) {
    std::uint8_t tr = get_rank(to);
    if (tr == 0 || tr == 7)
      test[to].piece =
          is_white_piece(mp) ? Piece::WHITE_QUEEN : Piece::BLACK_QUEEN;
  }
  if (piece_type(mp) == Piece::WHITE_PAWN && state.en_passant_target &&
      to == state.en_passant_target.value()) {
    test[file_rank_to_index(get_file(to), is_white_piece(mp)
                                              ? get_rank(to) - 1
                                              : get_rank(to) + 1)]
        .piece = Piece::NONE;
  }
  if (piece_type(mp) == Piece::WHITE_KING) {
    int ff = get_file(from), tf = get_file(to);
    if (ff == 4 && tf == 6) {
      std::uint8_t rr = get_rank(from);
      test[file_rank_to_index(5, rr)].piece =
          test[file_rank_to_index(7, rr)].piece;
      test[file_rank_to_index(7, rr)].piece = Piece::NONE;
    }
    if (ff == 4 && tf == 2) {
      std::uint8_t rr = get_rank(from);
      test[file_rank_to_index(3, rr)].piece =
          test[file_rank_to_index(0, rr)].piece;
      test[file_rank_to_index(0, rr)].piece = Piece::NONE;
    }
  }
  return !is_in_check(test, state.white_to_move);
}

// ============================================================
// IN-PLACE MOVE EXECUTION
// ============================================================
struct Move {
  std::uint8_t from, to;
  Piece promotion = Piece::NONE;
  int score = 0;
};
inline void apply_move(Board &board, GameState &state, Move m) {
  Piece mp = board[m.from].piece;
  bool is_white = is_white_piece(mp);
  state.last_from = m.from;
  state.last_to = m.to;
  if (piece_type(mp) == Piece::WHITE_KING) {
    int ff = get_file(m.from), tf = get_file(m.to);
    if (ff == 4 && tf == 6) {
      board[file_rank_to_index(5, get_rank(m.from))].piece =
          board[file_rank_to_index(7, get_rank(m.from))].piece;
      board[file_rank_to_index(7, get_rank(m.from))].piece = Piece::NONE;
    }
    if (ff == 4 && tf == 2) {
      board[file_rank_to_index(3, get_rank(m.from))].piece =
          board[file_rank_to_index(0, get_rank(m.from))].piece;
      board[file_rank_to_index(0, get_rank(m.from))].piece = Piece::NONE;
    }
    if (is_white)
      state.white_king_pos = m.to;
    else
      state.black_king_pos = m.to;
  }
  if (piece_type(mp) == Piece::WHITE_PAWN && state.en_passant_target &&
      m.to == state.en_passant_target.value()) {
    board[file_rank_to_index(get_file(m.to), is_white ? get_rank(m.to) - 1
                                                      : get_rank(m.to) + 1)]
        .piece = Piece::NONE;
  }
  board[m.to].piece = mp;
  board[m.from].piece = Piece::NONE;

  // === MODIFIED: Use m.promotion if provided, else default to Queen ===
  if (piece_type(mp) == Piece::WHITE_PAWN) {
    std::uint8_t tr = get_rank(m.to);
    if ((is_white && tr == 7) || (!is_white && tr == 0)) {
      board[m.to].piece =
          (m.promotion != Piece::NONE)
              ? m.promotion
              : (is_white ? Piece::WHITE_QUEEN : Piece::BLACK_QUEEN);
    }
    int rd = static_cast<int>(tr) - static_cast<int>(get_rank(m.from));
    if (std::abs(rd) == 2)
      state.en_passant_target =
          file_rank_to_index(get_file(m.from), is_white ? tr - 1 : tr + 1);
    else
      state.en_passant_target = std::nullopt;
  } else {
    state.en_passant_target = std::nullopt;
  }

  if (piece_type(mp) == Piece::WHITE_KING) {
    if (is_white) {
      state.white_kingside_castle = false;
      state.white_queenside_castle = false;
    } else {
      state.black_kingside_castle = false;
      state.black_queenside_castle = false;
    }
  } else if (piece_type(mp) == Piece::WHITE_ROOK) {
    if (m.from == 0)
      state.white_queenside_castle = false;
    if (m.from == 7)
      state.white_kingside_castle = false;
    if (m.from == 56)
      state.black_queenside_castle = false;
    if (m.from == 63)
      state.black_kingside_castle = false;
  }
  state.halfmove_clock =
      (piece_type(mp) == Piece::WHITE_PAWN || board[m.to].piece != Piece::NONE)
          ? 0
          : state.halfmove_clock + 1;
  state.white_to_move = !state.white_to_move;
  if (!is_white)
    state.fullmove_number++;
  state.in_check = is_in_check(board, state.white_to_move);
}

// ============================================================
// BOT LOGIC (OPTIMIZED MINIMAX)
// ============================================================
inline std::vector<Move> generate_moves(const Board &board,
                                        const GameState &state) {
  std::vector<Move> moves;
  moves.reserve(30);
  for (std::uint8_t from = 0; from < 64; ++from) {
    if (board[from].piece != Piece::NONE &&
        is_white_piece(board[from].piece) == state.white_to_move) {
      for (std::uint8_t to = 0; to < 64; ++to) {
        if (is_valid_move(board, from, to, state)) {
          int s = 0;
          if (board[to].piece != Piece::NONE)
            s += 10 * static_cast<int>(piece_type(board[to].piece)) -
                 static_cast<int>(piece_type(board[from].piece));
          moves.push_back({from, to, Piece::NONE, s});
        }
      }
    }
  }
  std::sort(moves.begin(), moves.end(),
            [](const Move &a, const Move &b) { return a.score > b.score; });
  return moves;
}
inline int minimax(Board &board, GameState &state, int depth, int alpha,
                   int beta, bool maximizing) {
  if (depth == 0)
    return evaluate_positional(board, state.white_to_move);
  auto moves = generate_moves(board, state);
  if (moves.empty()) {
    if (state.in_check)
      return maximizing ? -50000 + depth : 50000 - depth;
    return 0; // Stalemate
  }
  if (maximizing) {
    int maxEval = -1000000;
    for (const auto &m : moves) {
      Board next_b = board;
      GameState next_s = state;
      apply_move(next_b, next_s, m);
      int eval = minimax(next_b, next_s, depth - 1, alpha, beta, false);
      maxEval = std::max(maxEval, eval);
      alpha = std::max(alpha, eval);
      if (beta <= alpha)
        break;
    }
    return maxEval;
  } else {
    int minEval = 1000000;
    for (const auto &m : moves) {
      Board next_b = board;
      GameState next_s = state;
      apply_move(next_b, next_s, m);
      int eval = minimax(next_b, next_s, depth - 1, alpha, beta, true);
      minEval = std::min(minEval, eval);
      beta = std::min(beta, eval);
      if (beta <= alpha)
        break;
    }
    return minEval;
  }
}
inline Move get_best_move(Board &board, GameState &state) {
  auto moves = generate_moves(board, state);
  if (moves.empty())
    return {0, 0};
  Move best = moves[0];
  int bestEval = state.white_to_move ? -1000000 : 1000000;
  constexpr int DEPTH = 3;
  for (const auto &m : moves) {
    Board nb = board;
    GameState ns = state;
    apply_move(nb, ns, m);
    int eval =
        minimax(nb, ns, DEPTH - 1, -1000000, 1000000, !state.white_to_move);
    if ((state.white_to_move && eval > bestEval) ||
        (!state.white_to_move && eval < bestEval)) {
      bestEval = eval;
      best = m;
    }
  }
  return best;
}
inline void check_game_over(Board &board, GameState &state) {
  if (state.game_over)
    return;
  auto moves = generate_moves(board, state);
  if (moves.empty()) {
    state.game_over = true;
    state.result_message =
        state.in_check ? (state.white_to_move ? "Black wins by checkmate!"
                                              : "White wins by checkmate!")
                       : "Draw by stalemate!";
  }
}

// ============================================================
// UI & EVENT HANDLING
// ============================================================
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

inline void draw_board(const Board &board, sf::RenderWindow &window,
                       sf::RectangleShape &sq, sf::Sprite &ps,
                       const GameState &state, sf::RectangleShape &hl) {
  constexpr float SQ = 64.0f;
  for (std::uint8_t i = 0; i < 64; ++i) {
    sf::Color c =
        (board[i].color == SquareColor::DARK) ? DUSTY_GRAPE : LIGHT_YELLOW;
    if (state.selected_square == i)
      c = SELECTION_COLOR;
    else if (state.last_from == i || state.last_to == i)
      c = HIGHLIGHT_COLOR;
    else if (state.in_check &&
             ((state.white_to_move && board[i].piece == Piece::WHITE_KING) ||
              (!state.white_to_move && board[i].piece == Piece::BLACK_KING)))
      c = CHECK_COLOR;
    sq.setFillColor(c);
    sq.setPosition(board_index_to_pixel(i, state.player_is_white));
    window.draw(sq);
    if (board[i].piece != Piece::NONE) {
      ps.setTextureRect(get_texture_rect(board[i].piece));
      ps.setPosition(board_index_to_pixel(i, state.player_is_white));
      window.draw(ps);
    }
  }
  if (state.selected_square) {
    std::uint8_t f = state.selected_square.value();
    for (std::uint8_t t = 0; t < 64; ++t) {
      if (is_valid_move(board, f, t, state)) {
        sf::Vector2f p = board_index_to_pixel(t, state.player_is_white);
        if (board[t].piece != Piece::NONE) {
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

// === NEW: Promotion Dialog ===
inline Piece show_promotion_dialog(sf::RenderWindow &window,
                                   const sf::Font &font, bool is_white,
                                   const Board &board, const GameState &state,
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
    // Redraw scene + overlay
    window.clear(OCEAN_MIST);
    draw_board(board, window, sq, ps, state, hl);
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

// === MODIFIED: Detects promotion and returns it via pending_promo_move ===
inline void handle_events(sf::RenderWindow &window, Board &board,
                          GameState &state,
                          std::optional<Move> &pending_promo_move) {
  while (const auto ev = window.pollEvent()) {
    if (ev->is<sf::Event::Closed>())
      window.close();
    else if (const auto *mc = ev->getIf<sf::Event::MouseButtonPressed>()) {
      if (mc->button == sf::Mouse::Button::Left) {
        if (state.game_over) {
          bool wp = state.player_is_white;
          state = GameState{};
          state.player_is_white = wp;
          board = make_canonical_board();
          continue;
        }
        if (state.white_to_move != state.player_is_white)
          continue;
        auto idx = pixel_to_board_index(mc->position.x, mc->position.y,
                                        state.player_is_white);
        if (!idx) {
          state.selected_square = std::nullopt;
          continue;
        }
        std::uint8_t t = idx.value();
        if (!state.selected_square) {
          if (board[t].piece != Piece::NONE &&
              is_white_piece(board[t].piece) == state.white_to_move)
            state.selected_square = t;
          continue;
        }
        std::uint8_t f = state.selected_square.value();
        if (f == t) {
          state.selected_square = std::nullopt;
          continue;
        }
        if (is_valid_move(board, f, t, state)) {
          // Check if this is a promotion move
          bool is_promo = (piece_type(board[f].piece) == Piece::WHITE_PAWN) &&
                          ((state.white_to_move && get_rank(t) == 7) ||
                           (!state.white_to_move && get_rank(t) == 0));
          if (is_promo) {
            pending_promo_move =
                Move{f, t}; // Defer application until dialog choice
          } else {
            apply_move(board, state, {f, t});
          }
          state.selected_square = std::nullopt;
        } else if (board[t].piece != Piece::NONE &&
                   is_white_piece(board[t].piece) == state.white_to_move) {
          state.selected_square = t;
        } else {
          state.selected_square = std::nullopt;
        }
      }
    }
  }
}

int main() {
  sf::RenderWindow window{sf::VideoMode{{576, 576}}, "Chess vs Bot ",
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

  Board board = make_canonical_board();
  sf::RectangleShape sq{{64, 64}}, hl{{64, 64}};
  hl.setFillColor(sf::Color::Transparent);
  sf::Texture tex;
  if (!tex.loadFromMemory(chess_pieces, chess_pieces_size)) {
    std::cerr << "Texture load failed!\n";
    return 1;
  }
  sf::Sprite ps{tex};

  while (window.isOpen()) {
    std::optional<Move> pending_promo_move;
    handle_events(window, board, state, pending_promo_move);

    // === Handle Promotion Choice ===
    if (pending_promo_move) {
      Move m = pending_promo_move.value();
      bool is_white = state.white_to_move; // Still the side that just moved
      Piece promo = show_promotion_dialog(window, font, is_white, board, state,
                                          sq, ps, hl);
      if (promo != Piece::NONE) {
        m.promotion = promo;
        apply_move(board, state, m);
      }
    }

    if (!state.game_over)
      check_game_over(board, state);
    if (!state.game_over && state.white_to_move != state.player_is_white) {
      Move bm = get_best_move(board, state);
      apply_move(board, state, bm);
      check_game_over(board, state);
    }

    window.clear(OCEAN_MIST);
    draw_board(board, window, sq, ps, state, hl);
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
  return 0;
}
