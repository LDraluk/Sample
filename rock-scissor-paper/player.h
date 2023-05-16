#pragma once

#include "game_history.h"

#include "Current/bricks/util/random.h"

class IPlayerStrategy {
 public:
  virtual std::string GetNextMove(GameHistory const& game_history) const = 0;
  virtual ~IPlayerStrategy() = default;
};

class RandomPlayerStrategy : public IPlayerStrategy {
 public:
  virtual std::string GetNextMove(GameHistory const& game_history) const override {
    static_cast<void>(game_history);
    return GetRandomMove();
  }

  static std::string GetRandomMove() {
    static std::vector<std::string> moves {
      kRockPlayerInput,
      kPaperPlayerInput,
      kScissorsPlayerInput
    };
    return moves[RandomInt(0, moves.size() - 1)];
  }
};

class OtherPlayerLastMovePlayerStrategy : public IPlayerStrategy {
 public:
  OtherPlayerLastMovePlayerStrategy(bool is_player1) : is_player1_(is_player1) {}
  virtual std::string GetNextMove(GameHistory const& game_history) const override {
    std::string result;
    game_history.AccessHistoryReadOnly(
        [this, &result](GameHistory::game_history_t const& history) {
          if (history.empty()) {
            // If there's no history yet, choose random move.
            result = RandomPlayerStrategy::GetRandomMove();
          } else {
            // Otherwise return other player's move from the last round.
            auto const& last_move = history.back();
            result = is_player1_ ? last_move.Inputs.Player2 : last_move.Inputs.Player1;
          }
        }
    );
    return result;
  }

 private:
  bool const is_player1_;
};

class Player {
 public:
  Player(IPlayerStrategy const& strategy, GameHistory const& game_history)
      : destructing_(false),
        strategy_(strategy),
        game_history_(game_history),
        is_making_move_(false),
        thread_([this]() { Thread(); }) {}

  ~Player() {
    destructing_ = true;
    if (thread_.joinable()) {
      // Notify the thread that we're shutting down.
      {
        std::unique_lock<std::mutex> lock(signal_mutex_);
        signal_cv_.notify_one();
      }
      thread_.join();
    }
  }

  std::string GetNextMoveSynchronously() {
    // Acquire global lock.
    std::lock_guard<std::mutex> access_lock(access_mutex_);
    // Acquire signaling lock.
      
    std::unique_lock<std::mutex> signal_lock(signal_mutex_);
    // Set variable and notify processing thread.
    is_making_move_ = true;
    signal_cv_.notify_one();
    // Wait for the thread to make move.
    signal_cv_.wait(signal_lock, [this]() { return !is_making_move_; });
    return next_move_;
  }

 private: 
  void Thread() {
    while(!destructing_) {
       // Acquire signaling lock.
      std::unique_lock<std::mutex> lock(signal_mutex_);
 
      // Wait for either destruction or next move.
      signal_cv_.wait(lock, [this]() { return destructing_ || is_making_move_; });
      if (!destructing_) {
        // Make move and notify.
        next_move_ = strategy_.GetNextMove(game_history_);
        is_making_move_ = false;
        signal_cv_.notify_one();
      }
    }
  }

 private:
  std::atomic_bool destructing_;  
  IPlayerStrategy const& strategy_;
  GameHistory const& game_history_;
  std::mutex access_mutex_;
  std::mutex signal_mutex_;
  std::condition_variable signal_cv_;
  std::atomic_bool is_making_move_;
  std::string next_move_; 
  std::thread thread_;
};
