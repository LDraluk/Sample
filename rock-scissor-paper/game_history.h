#pragma once

#include "Current/current.h"


const std::string kRockPlayerInput = "rock";
const std::string kPaperPlayerInput = "paper";
const std::string kScissorsPlayerInput = "scissors";

struct InvalidPlayerInputException : current::Exception {
  using current::Exception::Exception;
};

CURRENT_STRUCT(PlayerInputs) {
  CURRENT_FIELD(Player1, std::string);
  CURRENT_FIELD(Player2, std::string);
  CURRENT_DEFAULT_CONSTRUCTOR(PlayerInputs) {}
  CURRENT_CONSTRUCTOR(PlayerInputs)(std::string player1, std::string player2) {
    if (!IsValidPlayerInput(player1)) {
      CURRENT_THROW(
          InvalidPlayerInputException(Printf("Invalid input for Player1: '%s'", player1.c_str()))
      );
    }
    if (!IsValidPlayerInput(player2)) {
      CURRENT_THROW(
          InvalidPlayerInputException(Printf("Invalid input for Player2: '%s'", player2.c_str()))
      );
    }
    Player1 = std::move(player1);
    Player2 = std::move(player2);
  }

  bool IsValidPlayerInput(std::string const& input) const {
    static std::set<std::string> const valid_inputs {
      kRockPlayerInput,
      kPaperPlayerInput,
      kScissorsPlayerInput
    };
    return valid_inputs.count(input);
  }
};

CURRENT_STRUCT(Move) {
  CURRENT_FIELD(Round, uint32_t, 0);
  CURRENT_FIELD(Winner, Optional<std::string>);
  CURRENT_FIELD(Inputs, PlayerInputs);
  CURRENT_DEFAULT_CONSTRUCTOR(Move) {}
  CURRENT_CONSTRUCTOR(Move)(uint32_t round, std::string player1_input, std::string player2_input)
      : Round(round), Inputs(std::move(player1_input), std::move(player2_input)) {
    SetWinnerFromCtor();
  }
static std::string DecideOnWinner(std::string const& player1_input,
                                  std::string const& player2_input) {
    if (player1_input != player2_input) {
        static std::map<std::pair<std::string, std::string>, std::string> const winner_map {
            {{kRockPlayerInput, kPaperPlayerInput}, "Player2"},
            {{kPaperPlayerInput, kRockPlayerInput}, "Player1"},
            {{kRockPlayerInput, kScissorsPlayerInput}, "Player1"},
            {{kScissorsPlayerInput, kRockPlayerInput}, "Player2"},
            {{kScissorsPlayerInput, kPaperPlayerInput}, "Player1"},
            {{kPaperPlayerInput, kScissorsPlayerInput}, "Player2"}
        };
        return winner_map.at({player1_input, player2_input});
    }
    return "";
}


 private:
  void SetWinnerFromCtor() {
    if (Inputs.Player1 != Inputs.Player2) {
      static std::map<std::pair<std::string, std::string>, std::string> const winner_map {
        {{kRockPlayerInput, kPaperPlayerInput}, "Player2"},
        {{kPaperPlayerInput, kRockPlayerInput}, "Player1"},
        {{kRockPlayerInput, kScissorsPlayerInput}, "Player1"},
        {{kScissorsPlayerInput, kRockPlayerInput}, "Player2"},
        {{kScissorsPlayerInput, kPaperPlayerInput}, "Player1"},
        {{kPaperPlayerInput, kScissorsPlayerInput}, "Player2"}
      };
      Winner = winner_map.at({Inputs.Player1, Inputs.Player2});
    }
  }
};


class GameHistory {
 public:
  using game_history_t = std::vector<Move>;

  void Clear() {
    AccessFromLockedSectionReadWrite(
        [](game_history_t& history) {
          history.clear();
        }
    );
  }

  void AddMove(
      uint32_t round,
      std::string const& player1_input,
      std::string const& player2_input
  ) {
    AccessFromLockedSectionReadWrite(
        [&](game_history_t& history) {
          history.emplace_back(round, player1_input, player2_input);
        }
    );
  }

  template <typename F>
  void AccessHistoryReadOnly(F&& f) const {
    AccessFromLockedSectionReadOnly([=](game_history_t const& history) { f(history); });
  }

  void DumpHistoryAsJSON(std::ostream& os) const {
    AccessFromLockedSectionReadOnly(
      [&](game_history_t const& history) {
        os << JSON(history) << std::endl;
      }
    );
  }

 private:
  template <typename F>
  void AccessFromLockedSectionReadOnly(F&& f) const {
    std::lock_guard<std::mutex> lock(mutex_);
    f(game_history_);
  }

  template <typename F>
  void AccessFromLockedSectionReadWrite(F&& f) {
    std::lock_guard<std::mutex> lock(mutex_);
    f(game_history_);
  }

 private:
  game_history_t game_history_;
  mutable std::mutex mutex_;
};
