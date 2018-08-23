#pragma once

#include <array>
#include <cstdint>

#include <immintrin.h>

#include <boost/container/static_vector.hpp>
#include <boost/functional/hash.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/numeric.hpp>

namespace barys {
  enum class piece_type: int {chick = 0, cat = 1, dog = 2, lion = 3, chicken = 4, power_up_cat = 5};

  inline auto promoted(const piece_type& piece_type) noexcept {
    switch (piece_type) {
    case barys::piece_type::chick:
      return barys::piece_type::chicken;

    case barys::piece_type::cat:
      return barys::piece_type::power_up_cat;

    default:
      return piece_type;
    }
  }

  inline auto demoted(const piece_type& piece_type) noexcept {
    switch (piece_type) {
    case barys::piece_type::chicken:
      return barys::piece_type::chick;

    case barys::piece_type::power_up_cat:
      return barys::piece_type::cat;

    default:
      return piece_type;
    }
  }

  static constexpr auto board_bits      = 0b00111111111111111111111111111111;
  static constexpr auto enemy_side_bits = 0b00000000000000000000001111111111;

  inline auto control(const piece_type& piece_type, int bit) noexcept {
    static constexpr std::uint32_t piece_controls[6] = {0b00001000000000000000000000000000,
                                                        0b00011100000000000000000001010000,
                                                        0b10011100000000000000000000100010,
                                                        0b10011100000000000000000001110010,
                                                        0b10011100000000000000000000100010,
                                                        0b10011100000000000000000000100010};

    static constexpr std::uint32_t control_masks[30] = {0b00000000000000000000000001100011,
                                                        0b00000000000000000000000011100111,
                                                        0b00000000000000000000000111001110,
                                                        0b00000000000000000000001110011100,
                                                        0b00000000000000000000001100011000,
                                                        0b00000000000000000000110001100011,
                                                        0b00000000000000000001110011100111,
                                                        0b00000000000000000011100111001110,
                                                        0b00000000000000000111001110011100,
                                                        0b00000000000000000110001100011000,
                                                        0b00000000000000011000110001100000,
                                                        0b00000000000000111001110011100000,
                                                        0b00000000000001110011100111000000,
                                                        0b00000000000011100111001110000000,
                                                        0b00000000000011000110001100000000,
                                                        0b00000000001100011000110000000000,
                                                        0b00000000011100111001110000000000,
                                                        0b00000000111001110011100000000000,
                                                        0b00000001110011100111000000000000,
                                                        0b00000001100011000110000000000000,
                                                        0b00000110001100011000000000000000,
                                                        0b00001110011100111000000000000000,
                                                        0b00011100111001110000000000000000,
                                                        0b00111001110011100000000000000000,
                                                        0b00110001100011000000000000000000,
                                                        0b00000110001100000000000000000000,
                                                        0b00001110011100000000000000000000,
                                                        0b00011100111000000000000000000000,
                                                        0b00111001110000000000000000000000,
                                                        0b00110001100000000000000000000000};

    return _rotl(piece_controls[static_cast<int>(piece_type)], bit) & control_masks[bit];
  }

  class action final {
    int _from_board;
    int _from_hand;
    int _to;

  public:
    constexpr action(int from_board, int from_hand, int to) noexcept: _from_board(from_board), _from_hand(from_hand), _to(to) {
      ;
    }

    constexpr action() noexcept: action(0, 0, 0) {
      ;
    }

    auto from_board() const noexcept {
      return _from_board;
    }

    auto from_hand() const noexcept {
      return _from_hand;
    }

    auto to() const noexcept {
      return _to;
    }
  };

  inline auto operator==(const action& action_1, const action& action_2) noexcept {
    return action_1.from_board() == action_2.from_board() && action_1.from_hand() == action_2.from_hand() && action_1.to() == action_2.to();
  }

  class state final {
    std::array<std::uint32_t, 6> _pieces_on_board;
    std::array<int,           4> _piece_counts_in_hand;
    std::array<std::uint32_t, 6> _enemy_pieces_on_board;
    std::array<int,           4> _enemy_piece_counts_in_hand;

  public:
    state(const std::array<std::uint32_t, 6>& pieces_on_board, const std::array<int, 4>& piece_counts_in_hand, const std::array<std::uint32_t, 6>& enemy_pieces_on_board, const std::array<int, 4>& enemy_piece_counts_in_hand) noexcept
      : _pieces_on_board(pieces_on_board), _piece_counts_in_hand(piece_counts_in_hand), _enemy_pieces_on_board(enemy_pieces_on_board), _enemy_piece_counts_in_hand(enemy_piece_counts_in_hand)
    {
      ;
    }

    state() noexcept: state({0b00000000000001110000000000000000,
                             0b00100010000000000000000000000000,
                             0b00010100000000000000000000000000,
                             0b00001000000000000000000000000000,
                             0b00000000000000000000000000000000,
                             0b00000000000000000000000000000000},
                            {0, 0, 0, 0},
                            {0b00000000000000000011100000000000,
                             0b00000000000000000000000000010001,
                             0b00000000000000000000000000001010,
                             0b00000000000000000000000000000100,
                             0b00000000000000000000000000000000,
                             0b00000000000000000000000000000000},
                            {0, 0, 0, 0})
    {
      ;
    }

    const auto& pieces_on_board() const noexcept {
      return _pieces_on_board;
    }

    const auto& piece_counts_in_hand() const noexcept {
      return _piece_counts_in_hand;
    }

    const auto& enemy_pieces_on_board() const noexcept {
      return _enemy_pieces_on_board;
    }

    const auto& enemy_piece_counts_in_hand() const noexcept {
      return _enemy_piece_counts_in_hand;
    }

    const auto& is_end() const noexcept {
      return _enemy_piece_counts_in_hand[static_cast<int>(piece_type::lion)];
    }

  private:
    auto chick_allowed_bits() const noexcept {
      auto result = 0u;

      const auto& chick_bits = pieces_on_board()[static_cast<int>(piece_type::chick)];

      for (auto i = 5; i <= 25; i += 5) {  // ひよこの後はダメ。シフトしてchick_bitsがまだ残っているかをチェックする方式より、無条件で5回まわす方が速かった。
        result |= chick_bits << i;
      }

      for (auto i = 5; i <= 25; i += 5) {  // ひよこの前はダメ。
        result |= chick_bits >> i;
      }

      result |= 0b00000000000000000000000000011111;  // 一番上の行は行き場所がなくなるのでダメ。
      result |= enemy_pieces_on_board()[static_cast<int>(piece_type::lion)] << 5;  // ライオンの前は「打ちひよこ詰め」の可能性があるのでダメ。「打ちひよこ詰め」の判定は時間がかかるので、これで代用します。

      return ~result;
    }

  public:
    auto actions() const noexcept {
      auto result = boost::container::static_vector<action, 4 * 3 + 14 * 5 + 12 * 8 + 1 * 25 + 2 * 28>();

      // moves.

      const auto& vacant_bits = ~boost::accumulate(pieces_on_board(), 0u, std::bit_or<std::uint32_t>());

      for (auto i = 0; i < 6; ++i) {
        for (auto piece_bits = pieces_on_board()[i]; piece_bits; piece_bits &= piece_bits - 1) {
          for (auto control_bits = control(static_cast<piece_type>(i), _tzcnt_u32(piece_bits)) & vacant_bits; control_bits; control_bits &= control_bits - 1) {  // TODO: _blsr_u32
            result.emplace_back(_tzcnt_u32(piece_bits), -1, _tzcnt_u32(control_bits));
          }
        }
      }

      // drops.

      const auto& enemy_vacant_bits  = ~boost::accumulate(enemy_pieces_on_board(), 0u, std::bit_or<std::uint32_t>());
      const auto& chick_allowed_bits = state::chick_allowed_bits();

      if (piece_counts_in_hand()[0]) {
        for (auto to_bits = board_bits & vacant_bits & enemy_vacant_bits & chick_allowed_bits; to_bits; to_bits &= to_bits - 1) {  // TODO: _blsr_u32
          result.emplace_back(-1, 0, _tzcnt_u32(to_bits));
        }
      }

      for (auto i = 1; i < 3; ++i) {
        if (piece_counts_in_hand()[i]) {
          for (auto to_bits = board_bits & vacant_bits & enemy_vacant_bits; to_bits; to_bits &= to_bits - 1) {  // TODO: _blsr_u32
            result.emplace_back(-1, i, _tzcnt_u32(to_bits));
          }
        }
      }

      return result;
    }

  private:
    auto reverse(std::uint32_t piece_bits) const noexcept {
      static constexpr std::uint8_t reversed[256] = {0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
                                                     0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
                                                     0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
                                                     0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
                                                     0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
                                                     0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
                                                     0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
                                                     0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
                                                     0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
                                                     0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
                                                     0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
                                                     0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
                                                     0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
                                                     0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
                                                     0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
                                                     0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};

      return (reversed[piece_bits       & 0x000000ff] << 24 |
              reversed[piece_bits >>  8 & 0x000000ff] << 16 |
              reversed[piece_bits >> 16 & 0x000000ff] <<  8 |
              reversed[piece_bits >> 24             ]);
    }

    auto reverse(std::array<std::uint32_t, 6>* pieces_on_board) const noexcept {
      for (auto i = 0; i < 6; ++i) {
        pieces_on_board->data()[i] = reverse(pieces_on_board->data()[i] << 2);
      }
    }

  public:
    auto next(const action& action) const noexcept {
      auto next_pieces_on_board       = std::array<std::uint32_t, 6>(pieces_on_board());
      auto next_piece_counts_in_hand  = std::array<int,           4>(piece_counts_in_hand());
      auto next_enemy_pieces_on_board = std::array<std::uint32_t, 6>(enemy_pieces_on_board());

      if (action.from_board() >= 0) {
        const auto& from = 1u << action.from_board();
        const auto& to   = 1u << action.to();

        for (auto i = 0; i < 6; ++i) {
          if (next_enemy_pieces_on_board[i] & to) {  // gccには、_bittestandresetがなかった……。
            next_enemy_pieces_on_board[i] &= ~to;
            next_piece_counts_in_hand[static_cast<int>(demoted(static_cast<piece_type>(i)))]++;

            break;
          }
        }

        for (auto i = 0; i < 6; ++i) {
          if (next_pieces_on_board[i] & from) {
            next_pieces_on_board[i] &= ~from;
            next_pieces_on_board[to & enemy_side_bits ? static_cast<int>(promoted(static_cast<piece_type>(i))) : i] |= to;

            break;
          }
        }

      } else {
        next_piece_counts_in_hand[action.from_hand()]--;
        next_pieces_on_board[action.from_hand()] |= 1u << action.to();
      }

      reverse(&next_pieces_on_board);
      reverse(&next_enemy_pieces_on_board);

      return state(next_enemy_pieces_on_board, enemy_piece_counts_in_hand(), next_pieces_on_board, next_piece_counts_in_hand);
    }
  };

  inline auto operator==(const state& state_1, const state& state_2) noexcept {
    for (auto i = 0; i < 6; ++i) {
      if (state_1.pieces_on_board()[i] != state_2.pieces_on_board()[i]) {
        return false;
      }
    }

    for (auto i = 0; i < 6; ++i) {
      if (state_1.enemy_pieces_on_board()[i] != state_2.enemy_pieces_on_board()[i]) {
        return false;
      }
    }

    for (auto i = 0; i < 4; ++i) {
      if (state_1.piece_counts_in_hand()[i] != state_2.piece_counts_in_hand()[i]) {
        return false;
      }
    }

    for (auto i = 0; i < 4; ++i) {
      if (state_1.enemy_piece_counts_in_hand()[i] != state_2.enemy_piece_counts_in_hand()[i]) {
        return false;
      }
    }

    return true;
  }
}

namespace std {
  template <>
  struct hash<barys::state> {
    auto operator()(const barys::state& state) const noexcept {
      auto result = static_cast<size_t>(0);

      for (auto i = 0; i < 6; ++i) {
        boost::hash_combine(result, state.pieces_on_board()[i]);
      }

      for (auto i = 0; i < 6; ++i) {
        boost::hash_combine(result, state.enemy_pieces_on_board()[i]);
      }

      for (auto i = 0; i < 4; ++i) {
        boost::hash_combine(result, state.piece_counts_in_hand()[i]);
      }

      for (auto i = 0; i < 4; ++i) {
        boost::hash_combine(result, state.enemy_piece_counts_in_hand()[i]);
      }

      return result;
    }
  };
}
