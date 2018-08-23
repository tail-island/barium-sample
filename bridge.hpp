#pragma once

#include <chrono>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/range/adaptors.hpp>

#include "alpha_beta.hpp"
#include "game.hpp"

namespace barys {
  class bridge final {
    int   _turn;
    state _state;

  public:
    bridge() noexcept: _turn(0), _state() {
      ;
    }

  private:
    auto from_barium_board(int index) const noexcept {
      const auto& y = index / 7 - 1;
      const auto& x = index % 7 - 1;

      if (_turn % 2 == 0) {
        return y * 5 + x;
      } else {
        return (5 - y) * 5 + (4 - x);
      }
    }

    auto from_barium_hand(int index) const noexcept {
      for (const auto& count: _state.piece_counts_in_hand() | boost::adaptors::indexed()) {
        if (index < count.value()) {
          return static_cast<int>(count.index());
        }

        index -= count.value();
      }

      return -1;
    }

    auto parse_message(const std::string& message) const noexcept {
      auto stream = std::stringstream(); stream << message;
      auto ptree  = boost::property_tree::ptree();

      boost::property_tree::read_json(stream, ptree);

      const auto& from_board = ptree.get_optional<int>("lastMove.fromBoard");
      const auto& from_hand  = ptree.get_optional<int>("lastMove.fromCaptured");
      const auto& to         = ptree.get_optional<int>("lastMove.to");

      return action(from_board ? from_barium_board(from_board.value()) : -1,
                    from_hand  ? from_barium_hand(from_hand.value())   : -1,
                    to         ? from_barium_board(to.value())         : -1);
    }

    auto to_barium_board(int index) const noexcept {
      const auto& y = index / 5;
      const auto& x = index % 5;

      if (_turn % 2 == 0) {
        return (y + 1) * 7 + (x + 1);
      } else {
        return (6 - y) * 7 + (5 - x);
      }
    }

    auto to_barium_hand(int index) const noexcept {
      int result = 0;

      for (const auto& count: _state.piece_counts_in_hand() | boost::adaptors::indexed()) {
        if (count.index() == index) {
          return result;
        }

        result += count.value();
      }

      return result;
    }

    auto encode_message(const action& action) const noexcept {
      auto stream = std::stringstream();
      auto ptree  = boost::property_tree::ptree();

      if (action.from_board() != -1) {
        ptree.put<int>("fromBoard", to_barium_board(action.from_board()));
      }

      if (action.from_hand() != -1) {
        ptree.put<int>("fromCaptured", to_barium_hand(action.from_hand()));
      }

      ptree.put<int>("to", to_barium_board(action.to()));

      boost::property_tree::write_json(stream, ptree);

      return stream.str();
    }

  public:
    auto operator()() noexcept {
      // TODO: リファクタリング。beastの使い方が分からなくて、サンプルをコピー＆ペーストした状態です。。。

      try {
        boost::asio::io_context io_context;
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket> websocket_stream{io_context};

        const auto& resolve_results = boost::asio::ip::tcp::resolver{io_context}.resolve("localhost", "8080");
        boost::asio::connect(websocket_stream.next_layer(), std::begin(resolve_results), std::end(resolve_results));

        websocket_stream.handshake("localhost", "/");

        for (;;) {
          auto buffer = boost::beast::multi_buffer();
          auto error  = boost::beast::error_code();

          websocket_stream.read(buffer, error);

          if (error == boost::beast::websocket::error::closed) {
            break;
          }

          const auto& last_action = parse_message(boost::beast::buffers_to_string(buffer.data()));

          if (last_action.to() >= 0) {
            _turn++;
            _state = _state.next(last_action);
          }

          const auto& next_action = alpha_beta(_state, std::chrono::system_clock::now() + std::chrono::milliseconds(14800))();

          websocket_stream.write(boost::asio::buffer(encode_message(next_action)));

          _turn++;
          _state = _state.next(next_action);
        }

      } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
      }
    }
  };
}
