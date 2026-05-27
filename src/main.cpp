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
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
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

// ============= SQUARE COLOR =============
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
  bool player_is_white = false; // NEW: which side the human plays
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
  for (std::uint8_t file = 0; file < 8; ++file)
    board[8 + file].piece = Piece::WHITE_PAWN;
  for (std::uint8_t file = 0; file < 8; ++file)
    board[48 + file].piece = Piece::BLACK_PAWN;
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
inline bool is_white_piece(Piece piece) noexcept {
  return piece != Piece::NONE && static_cast<std::uint8_t>(piece) < 6;
}
inline bool is_black_piece(Piece piece) noexcept {
  return static_cast<std::uint8_t>(piece) >= 6 && piece != Piece::NONE;
}
inline bool is_same_color(Piece a, Piece b) noexcept {
  if (a == Piece::NONE || b == Piece::NONE)
    return false;
  return is_white_piece(a) == is_white_piece(b);
}
inline Piece piece_type(Piece piece) noexcept {
  if (piece == Piece::NONE)
    return Piece::NONE;
  return static_cast<Piece>(static_cast<std::uint8_t>(piece) % 6);
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
  std::uint8_t piece_idx = static_cast<std::uint8_t>(piece);
  bool is_black = piece_idx >= 6;
  std::uint8_t relative_idx = piece_idx % 6;
  return {
      {relative_idx * PIECE_SIZE, is_black ? 1 * PIECE_SIZE : 0 * PIECE_SIZE},
      {PIECE_SIZE, PIECE_SIZE}};
}

// ============= MOUSE & COORDINATE HELPERS (FLIP-AWARE) =============
// When player is White, the board is flipped so White is at the bottom.
// We achieve this by inverting the rank in pixel<->board conversions.

inline std::uint8_t display_rank_to_board_rank(std::uint8_t display_rank,
                                               bool player_is_white) noexcept {
  return player_is_white ? (7 - display_rank) : display_rank;
}

inline std::uint8_t board_rank_to_display_rank(std::uint8_t board_rank,
                                               bool player_is_white) noexcept {
  return player_is_white ? (7 - board_rank) : board_rank;
}

inline std::optional<std::uint8_t>
pixel_to_board_index(int pixel_x, int pixel_y, bool player_is_white) noexcept {
  constexpr float SQUARE_SIZE = 64.0f;
  constexpr float MARGIN = 32.0f;
  constexpr float BOARD_END = MARGIN + 8.0f * SQUARE_SIZE;
  if (pixel_x < MARGIN || pixel_y < MARGIN || pixel_x >= BOARD_END ||
      pixel_y >= BOARD_END)
    return std::nullopt;
  std::uint8_t file =
      static_cast<std::uint8_t>((pixel_x - MARGIN) / SQUARE_SIZE);
  std::uint8_t display_rank =
      static_cast<std::uint8_t>((pixel_y - MARGIN) / SQUARE_SIZE);
  if (file >= 8 || display_rank >= 8)
    return std::nullopt;
  std::uint8_t rank = display_rank_to_board_rank(display_rank, player_is_white);
  return rank * 8 + file;
}

inline sf::Vector2f board_index_to_pixel(std::uint8_t idx,
                                         bool player_is_white) noexcept {
  constexpr float SQUARE_SIZE = 64.0f;
  constexpr float MARGIN = 32.0f;
  std::uint8_t board_rank = get_rank(idx);
  std::uint8_t display_rank =
      board_rank_to_display_rank(board_rank, player_is_white);
  return {get_file(idx) * SQUARE_SIZE + MARGIN,
          display_rank * SQUARE_SIZE + MARGIN};
}

// ============================================================
// MOVE VALIDATION ENGINE
// ============================================================

inline bool is_path_clear(const Board &board, std::uint8_t from,
                          std::uint8_t to) {
  std::uint8_t from_file = get_file(from), from_rank = get_rank(from);
  std::uint8_t to_file = get_file(to), to_rank = get_rank(to);
  int file_step = (to_file > from_file) ? 1 : (to_file < from_file) ? -1 : 0;
  int rank_step = (to_rank > from_rank) ? 1 : (to_rank < from_rank) ? -1 : 0;
  std::uint8_t cf = from_file + file_step, cr = from_rank + rank_step;
  while (cf != to_file || cr != to_rank) {
    if (board[file_rank_to_index(cf, cr)].piece != Piece::NONE)
      return false;
    cf += file_step;
    cr += rank_step;
  }
  return true;
}

inline bool is_square_attacked(const Board &board, std::uint8_t sq,
                               bool by_white) {
  std::uint8_t sq_file = get_file(sq), sq_rank = get_rank(sq);

  // Pawn attacks
  int pawn_dir = by_white ? -1 : 1;
  std::uint8_t attack_rank = sq_rank + pawn_dir;
  if (attack_rank < 8) {
    if (sq_file > 0) {
      Piece p = board[file_rank_to_index(sq_file - 1, attack_rank)].piece;
      if (p != Piece::NONE && is_white_piece(p) == by_white &&
          piece_type(p) == Piece::WHITE_PAWN)
        return true;
    }
    if (sq_file < 7) {
      Piece p = board[file_rank_to_index(sq_file + 1, attack_rank)].piece;
      if (p != Piece::NONE && is_white_piece(p) == by_white &&
          piece_type(p) == Piece::WHITE_PAWN)
        return true;
    }
  }

  // Knight attacks
  constexpr int knight_moves[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                                      {1, -2},  {1, 2},  {2, -1},  {2, 1}};
  for (const auto &m : knight_moves) {
    int nf = sq_file + m[0], nr = sq_rank + m[1];
    if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
      Piece p = board[file_rank_to_index(nf, nr)].piece;
      if (p != Piece::NONE && is_white_piece(p) == by_white &&
          piece_type(p) == Piece::WHITE_KNIGHT)
        return true;
    }
  }

  // King attacks
  for (int df = -1; df <= 1; ++df) {
    for (int dr = -1; dr <= 1; ++dr) {
      if (df == 0 && dr == 0)
        continue;
      int nf = sq_file + df, nr = sq_rank + dr;
      if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
        Piece p = board[file_rank_to_index(nf, nr)].piece;
        if (p != Piece::NONE && is_white_piece(p) == by_white &&
            piece_type(p) == Piece::WHITE_KING)
          return true;
      }
    }
  }

  // Rook/Queen (straight)
  constexpr int rook_dirs[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
  for (const auto &d : rook_dirs) {
    int cf = sq_file + d[0], cr = sq_rank + d[1];
    while (cf >= 0 && cf < 8 && cr >= 0 && cr < 8) {
      Piece p = board[file_rank_to_index(cf, cr)].piece;
      if (p != Piece::NONE) {
        if (is_white_piece(p) == by_white &&
            (piece_type(p) == Piece::WHITE_ROOK ||
             piece_type(p) == Piece::WHITE_QUEEN))
          return true;
        break;
      }
      cf += d[0];
      cr += d[1];
    }
  }

  // Bishop/Queen (diagonal)
  constexpr int bishop_dirs[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
  for (const auto &d : bishop_dirs) {
    int cf = sq_file + d[0], cr = sq_rank + d[1];
    while (cf >= 0 && cf < 8 && cr >= 0 && cr < 8) {
      Piece p = board[file_rank_to_index(cf, cr)].piece;
      if (p != Piece::NONE) {
        if (is_white_piece(p) == by_white &&
            (piece_type(p) == Piece::WHITE_BISHOP ||
             piece_type(p) == Piece::WHITE_QUEEN))
          return true;
        break;
      }
      cf += d[0];
      cr += d[1];
    }
  }
  return false;
}

inline bool is_in_check(const Board &board, bool white_king) {
  std::uint8_t king_pos = 0;
  for (std::uint8_t i = 0; i < 64; ++i) {
    if (board[i].piece ==
        (white_king ? Piece::WHITE_KING : Piece::BLACK_KING)) {
      king_pos = i;
      break;
    }
  }
  return is_square_attacked(board, king_pos, !white_king);
}

inline Board make_move(const Board &board, std::uint8_t from, std::uint8_t to,
                       Piece promotion = Piece::NONE) {
  Board result = board;
  Piece moving = result[from].piece;
  result[to].piece = moving;
  result[from].piece = Piece::NONE;
  if (piece_type(moving) == Piece::WHITE_PAWN) {
    if (get_rank(to) == 0 || get_rank(to) == 7) {
      result[to].piece = (promotion != Piece::NONE)
                             ? promotion
                             : (is_white_piece(moving) ? Piece::WHITE_QUEEN
                                                       : Piece::BLACK_QUEEN);
    }
  }
  return result;
}

inline bool is_valid_pawn_move(const Board &board, std::uint8_t from,
                               std::uint8_t to, const GameState &state,
                               bool is_white) {
  std::uint8_t from_file = get_file(from), from_rank = get_rank(from);
  std::uint8_t to_file = get_file(to), to_rank = get_rank(to);
  int direction = is_white ? 1 : -1;
  int rank_diff = static_cast<int>(to_rank) - static_cast<int>(from_rank);
  int file_diff = static_cast<int>(to_file) - static_cast<int>(from_file);

  if (file_diff == 0) {
    if (rank_diff == direction)
      return board[to].piece == Piece::NONE;
    if (rank_diff == 2 * direction) {
      std::uint8_t mid = file_rank_to_index(from_file, from_rank + direction);
      return board[to].piece == Piece::NONE &&
             board[mid].piece == Piece::NONE &&
             ((is_white && from_rank == 1) || (!is_white && from_rank == 6));
    }
  }
  if (std::abs(file_diff) == 1 && rank_diff == direction) {
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
  if (fd != rd || fd == 0)
    return false;
  return is_path_clear(board, from, to);
}

inline bool is_valid_rook_move(const Board &board, std::uint8_t from,
                               std::uint8_t to) {
  bool same_file = (get_file(from) == get_file(to));
  bool same_rank = (get_rank(from) == get_rank(to));
  if ((!same_file && !same_rank) || (same_file && same_rank))
    return false;
  return is_path_clear(board, from, to);
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

  std::uint8_t king_rank = is_white ? 0 : 7;
  std::uint8_t king_pos = file_rank_to_index(4, king_rank);
  if (board[king_pos].piece !=
      (is_white ? Piece::WHITE_KING : Piece::BLACK_KING))
    return false;
  if (is_in_check(board, is_white))
    return false;

  std::uint8_t rook_file = kingside ? 7 : 0;
  std::uint8_t rook_pos = file_rank_to_index(rook_file, king_rank);
  Piece expected_rook = is_white ? Piece::WHITE_ROOK : Piece::BLACK_ROOK;
  if (board[rook_pos].piece != expected_rook)
    return false;

  int step = kingside ? 1 : -1;
  for (int f = 4 + step; f != rook_file; f += step) {
    if (board[file_rank_to_index(f, king_rank)].piece != Piece::NONE)
      return false;
  }

  for (int f = 4 + step; (kingside ? f <= 6 : f >= 2); f += step) {
    if (is_square_attacked(board, file_rank_to_index(f, king_rank), !is_white))
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
  Piece moving_piece = board[from].piece;
  if (moving_piece == Piece::NONE)
    return false;
  bool is_white = is_white_piece(moving_piece);
  if (is_white != state.white_to_move)
    return false;

  Piece target = board[to].piece;
  if (target != Piece::NONE && is_same_color(moving_piece, target))
    return false;

  Piece type = piece_type(moving_piece);
  bool valid = false;

  switch (type) {
  case Piece::WHITE_PAWN:
    valid = is_valid_pawn_move(board, from, to, state, is_white);
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
        valid = can_castle(board, state, is_white, true);
      else if (get_file(to) == 2)
        valid = can_castle(board, state, is_white, false);
    }
    break;
  default:
    valid = false;
  }

  if (!valid)
    return false;

  // Test if move leaves king in check
  Board test = make_move(board, from, to);
  return !is_in_check(test, is_white);
}

// ============================================================
// EVENT HANDLING & MOVE EXECUTION
// ============================================================

inline void update_castling_rights(GameState &state, std::uint8_t from,
                                   Piece moved) {
  if (moved == Piece::WHITE_KING) {
    state.white_kingside_castle = false;
    state.white_queenside_castle = false;
  } else if (moved == Piece::BLACK_KING) {
    state.black_kingside_castle = false;
    state.black_queenside_castle = false;
  } else if (moved == Piece::WHITE_ROOK) {
    if (from == 0)
      state.white_queenside_castle = false;
    if (from == 7)
      state.white_kingside_castle = false;
  } else if (moved == Piece::BLACK_ROOK) {
    if (from == 56)
      state.black_queenside_castle = false;
    if (from == 63)
      state.black_kingside_castle = false;
  }
}

inline void execute_move(Board &board, GameState &state, std::uint8_t from,
                         std::uint8_t to) {
  Piece moving_piece = board[from].piece;
  bool is_white = is_white_piece(moving_piece);

  // Handle castling
  if (piece_type(moving_piece) == Piece::WHITE_KING) {
    std::uint8_t from_file = get_file(from), to_file = get_file(to);
    std::uint8_t rank = get_rank(from);
    if (from_file == 4 && to_file == 6) {
      std::uint8_t rook_from = file_rank_to_index(7, rank);
      std::uint8_t rook_to = file_rank_to_index(5, rank);
      board[rook_to].piece = board[rook_from].piece;
      board[rook_from].piece = Piece::NONE;
    } else if (from_file == 4 && to_file == 2) {
      std::uint8_t rook_from = file_rank_to_index(0, rank);
      std::uint8_t rook_to = file_rank_to_index(3, rank);
      board[rook_to].piece = board[rook_from].piece;
      board[rook_from].piece = Piece::NONE;
    }
    if (is_white)
      state.white_king_pos = to;
    else
      state.black_king_pos = to;
  }

  // Handle en passant capture
  if (piece_type(moving_piece) == Piece::WHITE_PAWN &&
      state.en_passant_target && to == state.en_passant_target.value()) {
    std::uint8_t captured_rank = is_white ? get_rank(to) - 1 : get_rank(to) + 1;
    board[file_rank_to_index(get_file(to), captured_rank)].piece = Piece::NONE;
  }

  // Set en passant target
  state.en_passant_target = std::nullopt;
  if (piece_type(moving_piece) == Piece::WHITE_PAWN) {
    int rank_diff =
        static_cast<int>(get_rank(to)) - static_cast<int>(get_rank(from));
    if (std::abs(rank_diff) == 2) {
      std::uint8_t ep_rank = is_white ? get_rank(from) + 1 : get_rank(from) - 1;
      state.en_passant_target = file_rank_to_index(get_file(from), ep_rank);
    }
  }

  // Update clocks
  if (piece_type(moving_piece) == Piece::WHITE_PAWN ||
      board[to].piece != Piece::NONE)
    state.halfmove_clock = 0;
  else
    state.halfmove_clock++;
  if (!is_white)
    state.fullmove_number++;

  // Execute move
  board[to].piece = moving_piece;
  board[from].piece = Piece::NONE;

  // Promotion
  if (piece_type(moving_piece) == Piece::WHITE_PAWN) {
    if ((is_white && get_rank(to) == 7) || (!is_white && get_rank(to) == 0)) {
      board[to].piece = is_white ? Piece::WHITE_QUEEN : Piece::BLACK_QUEEN;
    }
  }

  update_castling_rights(state, from, moving_piece);
  state.last_from = from;
  state.last_to = to;
  state.white_to_move = !state.white_to_move;
  state.in_check = is_in_check(board, state.white_to_move);
}

inline void handle_events(sf::RenderWindow &window, Board &board,
                          GameState &state) {
  while (const auto event = window.pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
      window.close();
    } else if (const auto *mouse_click =
                   event->getIf<sf::Event::MouseButtonPressed>()) {
      if (mouse_click->button == sf::Mouse::Button::Left) {
        auto target_idx = pixel_to_board_index(mouse_click->position.x,
                                               mouse_click->position.y,
                                               state.player_is_white);
        if (!target_idx) {
          state.selected_square = std::nullopt;
          continue;
        }

        std::uint8_t target = target_idx.value();
        if (!state.selected_square) {
          if (board[target].piece != Piece::NONE &&
              is_white_piece(board[target].piece) == state.white_to_move) {
            state.selected_square = target;
          }
          continue;
        }

        std::uint8_t from = state.selected_square.value();
        if (from == target) {
          state.selected_square = std::nullopt;
          continue;
        }

        if (is_valid_move(board, from, target, state)) {
          execute_move(board, state, from, target);
          state.selected_square = std::nullopt;
        } else if (board[target].piece != Piece::NONE &&
                   is_white_piece(board[target].piece) == state.white_to_move) {
          state.selected_square = target;
        } else {
          state.selected_square = std::nullopt;
        }
      }
    }
  }
}

// ============================================================
// SIDE SELECTION SCREEN
// ============================================================

/////////////// Pay attention here and replace with paths on your system for
///fonts ////////////
sf::Font k{sf::Font("/usr/share/fonts/TTF/Hack-BoldItalic.ttf")};
sf::Text nothingToSee{k, "something", 30};

struct Button {
  sf::RectangleShape shape;
  sf::Text text{nothingToSee};
  bool hovered = false;

  Button(const sf::Font &font, const std::string &label, sf::Vector2f position,
         sf::Vector2f size) {
    shape.setSize(size);
    shape.setPosition(position);
    shape.setFillColor(BUTTON_COLOR);
    text.setFont(font);
    text.setString(label);
    text.setCharacterSize(28);
    text.setFillColor(BUTTON_TEXT_COLOR);
    // Center text in button
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin({textBounds.position.x + textBounds.size.x / 2.0f,
                    textBounds.position.y + textBounds.size.y / 2.0f});
    text.setPosition({position.x + size.x / 2.0f, position.y + size.y / 2.0f});
  }

  bool contains(sf::Vector2f point) const {
    return shape.getGlobalBounds().contains(point);
  }

  void setHovered(bool h) {
    hovered = h;
    shape.setFillColor(hovered ? BUTTON_HOVER_COLOR : BUTTON_COLOR);
  }

  void draw(sf::RenderWindow &window) const {
    window.draw(shape);
    window.draw(text);
  }
};

enum class AppState { SIDE_SELECTION, PLAYING };

inline bool run_side_selection(sf::RenderWindow &window, GameState &state,
                               const sf::Font &font) {
  AppState app_state = AppState::SIDE_SELECTION;

  // Title text
  sf::Text title(font, "Choose Your Side", 42);
  title.setFillColor(TITLE_COLOR);
  sf::FloatRect titleBounds = title.getLocalBounds();
  title.setOrigin({titleBounds.position.x + titleBounds.size.x / 2.0f,
                   titleBounds.position.y + titleBounds.size.y / 2.0f});
  title.setPosition({288.0f, 150.0f});

  // Subtitle
  sf::Text subtitle(font, "White pieces move first", 18);
  subtitle.setFillColor(sf::Color(200, 200, 200));
  sf::FloatRect subBounds = subtitle.getLocalBounds();
  subtitle.setOrigin({subBounds.position.x + subBounds.size.x / 2.0f,
                      subBounds.position.y + subBounds.size.y / 2.0f});
  subtitle.setPosition({288.0f, 200.0f});

  // Buttons
  Button whiteBtn(font, "Play as White", {188.0f, 260.0f}, {200.0f, 60.0f});
  Button blackBtn(font, "Play as Black", {188.0f, 350.0f}, {200.0f, 60.0f});

  while (window.isOpen() && app_state == AppState::SIDE_SELECTION) {
    // Event handling
    while (const auto event = window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        window.close();
        return false;
      }

      if (const auto *mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseMoved->position.x),
                              static_cast<float>(mouseMoved->position.y));
        whiteBtn.setHovered(whiteBtn.contains(mousePos));
        blackBtn.setHovered(blackBtn.contains(mousePos));
      }

      if (const auto *mouseClick =
              event->getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseClick->button == sf::Mouse::Button::Left) {
          sf::Vector2f mousePos(static_cast<float>(mouseClick->position.x),
                                static_cast<float>(mouseClick->position.y));
          if (whiteBtn.contains(mousePos)) {
            state.player_is_white = true;
            app_state = AppState::PLAYING;
          } else if (blackBtn.contains(mousePos)) {
            state.player_is_white = false;
            app_state = AppState::PLAYING;
          }
        }
      }
    }

    // Render
    window.clear(DUSTY_GRAPE);
    window.draw(title);
    window.draw(subtitle);
    whiteBtn.draw(window);
    blackBtn.draw(window);
    window.display();
  }

  return window.isOpen();
}

// ============================================================
// RENDERING
// ============================================================

inline void draw_board(const Board &board, sf::RenderWindow &window,
                       sf::RectangleShape &square, sf::Sprite &piece_sprite,
                       const GameState &state, sf::RectangleShape &highlight) {
  constexpr float SQUARE_SIZE = 64.0f;
  constexpr float MARGIN = 32.0f;

  for (std::uint8_t idx = 0; idx < 64; ++idx) {
    const SquareInfo &info = board[idx];
    sf::Color square_color =
        (info.color == SquareColor::DARK) ? DUSTY_GRAPE : LIGHT_YELLOW;

    if (state.selected_square == idx)
      square_color = SELECTION_COLOR;
    else if (state.last_from == idx || state.last_to == idx)
      square_color = HIGHLIGHT_COLOR;
    else if (state.in_check) {
      Piece piece = info.piece;
      if ((state.white_to_move && piece == Piece::WHITE_KING) ||
          (!state.white_to_move && piece == Piece::BLACK_KING))
        square_color = CHECK_COLOR;
    }

    square.setFillColor(square_color);
    sf::Vector2f pos = board_index_to_pixel(idx, state.player_is_white);
    square.setPosition(pos);
    window.draw(square);

    if (info.piece != Piece::NONE) {
      piece_sprite.setTextureRect(get_texture_rect(info.piece));
      piece_sprite.setPosition(pos);
      window.draw(piece_sprite);
    }
  }

  // Valid move indicators
  if (state.selected_square) {
    std::uint8_t from = state.selected_square.value();
    for (std::uint8_t to = 0; to < 64; ++to) {
      if (is_valid_move(board, from, to, state)) {
        sf::Vector2f pos = board_index_to_pixel(to, state.player_is_white);
        if (board[to].piece != Piece::NONE) {
          highlight.setPosition({pos.x + 4, pos.y + 4});
          highlight.setSize({SQUARE_SIZE - 8, SQUARE_SIZE - 8});
          highlight.setFillColor(sf::Color::Transparent);
          highlight.setOutlineColor(VALID_MOVE_COLOR);
          highlight.setOutlineThickness(4);
          window.draw(highlight);
        } else {
          highlight.setPosition(
              {pos.x + SQUARE_SIZE / 2 - 8, pos.y + SQUARE_SIZE / 2 - 8});
          highlight.setSize({16, 16});
          highlight.setFillColor(VALID_MOVE_COLOR);
          highlight.setOutlineThickness(0);
          window.draw(highlight);
        }
      }
    }
  }
}

inline sf::RectangleShape create_square() {
  return sf::RectangleShape{{64.0f, 64.0f}};
}
inline sf::RectangleShape create_highlight_square() {
  auto sq = sf::RectangleShape{{64.0f, 64.0f}};
  sq.setFillColor(sf::Color::Transparent);
  return sq;
}
inline sf::RenderWindow create_window() {
  return sf::RenderWindow{sf::VideoMode{{576, 576}},
                          "Chess with Move Validation", sf::Style::Close};
}

// ============= MAIN =============
int main() {
  auto window = create_window();

  // Load font for UI
  sf::Font font;
  bool font_loaded = false;

  // Try to load system fonts
  /////////////// Pay attention here and add with paths on your system for fonts
  ///////////////
  const char *fontPaths[] = {
      "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
      "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
      "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
      "/usr/share/fonts/TTF/DejaVuSans.ttf",
      "/System/Library/Fonts/Helvetica.ttc",
      "C://Windows/Fonts/arial.ttf",
      "/usr/share/fonts/TTF/Hack-BoldItalic.ttf",
      nullptr};

  for (int i = 0; fontPaths[i] != nullptr; ++i) {
    if (font.openFromFile(fontPaths[i])) {
      font_loaded = true;
      break;
    }
  }

  if (!font_loaded) {
    std::cerr << "Warning: Could not load font. UI may not display correctly."
              << std::endl;
  }

  GameState state{};

  // Run side selection screen
  if (font_loaded) {
    if (!run_side_selection(window, state, font)) {
      return 0; // Window closed during selection
    }
  } else {
    // Default to White if no font available
    state.player_is_white = true;
  }

  Board board = make_canonical_board();

  auto square = create_square();
  auto highlight_square = create_highlight_square();

  sf::Texture pieces_texture;
  if (!pieces_texture.loadFromMemory(chess_pieces, chess_pieces_size)) {
    std::cerr << "Failed to load piece textures!" << std::endl;
    return 1;
  }

  sf::Sprite piece_sprite{pieces_texture};

  while (window.isOpen()) {
    handle_events(window, board, state);
    window.clear(OCEAN_MIST);
    draw_board(board, window, square, piece_sprite, state, highlight_square);
    window.display();
  }

  return 0;
}
