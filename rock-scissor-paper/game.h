#pragma once

#include "player.h"

class Game {
 public:
  Game(IPlayerStrategy const& player1_strategy, IPlayerStrategy const& player2_strategy)
      : player1_(player1_strategy, game_history_),
        player2_(player2_strategy, game_history_) {}

  void RunGameSynchronously(std::string const& output_file, int n_rounds) {
    if (n_rounds <= 0) {
      return;
    }
    std::ofstream os(output_file);
    if (!os.good()) {
      return;
    }
    for (int round = 1; round <= n_rounds; ++round) {
      auto const player1_move = player1_.GetNextMoveSynchronously();
      auto const player2_move = player2_.GetNextMoveSynchronously();
      game_history_.AddMove(round, player1_move, player2_move);
    }
    game_history_.DumpHistoryAsJSON(os);
    std::cout << output_file << std::endl;
  }

 private:
  GameHistory game_history_;
  Player player1_;
  Player player2_;
};
