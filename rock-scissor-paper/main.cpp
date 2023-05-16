//
//  main.cpp
//  Rock
//
//  Created by Leah Draluk on 12/2/18.
//  Copyright © 2018 Leah Draluk. All rights reserved.
//

#include "game.h"

#include "Current/3rdparty/gtest/gtest-main.h"
// check number of rounds is 100
TEST(Game1, Smoke) {
    RandomPlayerStrategy random_strategy;
    OtherPlayerLastMovePlayerStrategy other_player_move_strategy_for_player2(false);
    Game game(random_strategy, other_player_move_strategy_for_player2);
    const std::string out_file = "game1.json";
    game.RunGameSynchronously(out_file, 100);
    auto const file_content = current::FileSystem::ReadFileAsString(out_file);
    std::cout << file_content << std::endl;
    auto const game_history = ParseJSON<GameHistory::game_history_t>(file_content);
    EXPECT_EQ(100u, game_history.size());
}
//check thats other player strategy is correct
TEST(Game2, Smoke) {
    RandomPlayerStrategy random_strategy;
    OtherPlayerLastMovePlayerStrategy other_player_move_strategy_for_player2(false);
    Game game(random_strategy, other_player_move_strategy_for_player2);
    const std::string out_file = "game2.json";
    game.RunGameSynchronously(out_file, 100);
    auto const file_content = current::FileSystem::ReadFileAsString(out_file);
    std::cout << file_content << std::endl;
    auto const game_history = ParseJSON<GameHistory::game_history_t>(file_content);
    std::string prev;
    for (auto game : game_history) {
        if (game.Round == 1) {
            prev = game.Inputs.Player1;
            continue;
        }
        EXPECT_EQ(prev,game.Inputs.Player2);
        prev = game.Inputs.Player1;
    }
}
TEST(Game3, Smoke) {
    RandomPlayerStrategy random_strategy;
    OtherPlayerLastMovePlayerStrategy other_player_move_strategy_for_player2(false);
    Game game(random_strategy, other_player_move_strategy_for_player2);
    const std::string out_file = "game3.json";
    game.RunGameSynchronously(out_file, 100);
    auto const file_content = current::FileSystem::ReadFileAsString(out_file);
    std::cout << file_content << std::endl;
    auto const game_history = ParseJSON<GameHistory::game_history_t>(file_content);
    for (auto game : game_history) {
        if (Exists(game.Winner)) {
            EXPECT_EQ(Value(game.Winner),game.DecideOnWinner(game.Inputs.Player1,game.Inputs.Player2));
        } else {
            EXPECT_EQ(game.Inputs.Player1,game.Inputs.Player2);
        }
    }
}
