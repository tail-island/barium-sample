#pragma once

#include <array>
#include <chrono>

#include <nmmintrin.h>

#include "game.hpp"

namespace barys {
  class alpha_beta final {
    const std::chrono::system_clock::time_point& _time_limit;

  public:
    alpha_beta(const std::chrono::system_clock::time_point& time_limit) noexcept: _time_limit(time_limit) {
      ;
    }

  private:
    auto board_score(const std::array<std::uint32_t, 6>& pieces_on_board) const noexcept {
      return static_cast<int>(_mm_popcnt_u32(pieces_on_board[static_cast<int>(piece_type::chick)])        *  100 +
                              _mm_popcnt_u32(pieces_on_board[static_cast<int>(piece_type::cat)])          * 1000 +
                              _mm_popcnt_u32(pieces_on_board[static_cast<int>(piece_type::dog)])          * 1200 +
                              _mm_popcnt_u32(pieces_on_board[static_cast<int>(piece_type::chicken)])      * 1200 +
                              _mm_popcnt_u32(pieces_on_board[static_cast<int>(piece_type::power_up_cat)]) * 1200);
    };

    auto hand_score(const std::array<int, 4>& piece_counts_in_hand) const noexcept {
      return (piece_counts_in_hand[static_cast<int>(piece_type::chick)] *  100 +
              piece_counts_in_hand[static_cast<int>(piece_type::cat)]   * 1000 +
              piece_counts_in_hand[static_cast<int>(piece_type::dog)]   * 1200);
    };

    auto evaluate(const state& state) const noexcept {
      return board_score(state.pieces_on_board()) - board_score(state.enemy_pieces_on_board()) + hand_score(state.piece_counts_in_hand()) - hand_score(state.enemy_piece_counts_in_hand());
    }

    __forceinline auto score(const state& state, int depth, int alpha, int beta) const noexcept {
      if (std::chrono::system_clock::now() > _time_limit) {
        return alpha;
      }

      if (state.is_end()) {
        return -100000;
      }

      if (depth == 0) {
        return evaluate(state);
      }

      for (const auto& action: state.actions()) {
        const auto& score = -alpha_beta::score(state.next(action), depth - 1, -beta, -alpha);

        if (score > alpha) {
          alpha = score;
        }

        if (alpha >= beta) {
          return alpha;
        }
      }

      return alpha;
    }

  public:
    auto operator()(const state& state) const noexcept {
      auto result = action();

      auto alpha = -1000000;

      for (const auto& action: state.actions()) {
        const auto& score = -alpha_beta::score(state.next(action), 7, -1000000, -alpha);

        if (score > alpha) {
          alpha = score;

          result = action;
        }
      }

      return result;
    }
  };
}
