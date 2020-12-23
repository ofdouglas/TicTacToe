#include <iostream>

#include "tictactoe.h"

void print_usage() {
    std::cout << "Usage: tictactoe [X] [X] [N]" << std::endl;
    std::cout << " where X is one of {h, H, c, C} (human or computer, default human)" << std::endl;
    std::cout << " and N is the board dimension (default 3, range [3, 10])" << std::endl;
}

using namespace TicTacToe;

int main(int argc, char* argv[]) {
    constexpr int min_dimensions = 3;
    constexpr int max_dimensions = 10;

    Player* players[2];
    int player_index = 0;
    int dimensions = 3;

    for (int i = 1; i < argc; i++) {
        if (argv[i] == std::string("h") || argv[i] == std::string("H")) {
            players[player_index++] = new HumanPlayer();
        } else if (argv[i] == std::string("c") || argv[i] == std::string("C")) {
            players[player_index++] = new ComputerPlayer();
        } else {
            try {
                dimensions = std::stoi(argv[i]);
                if (dimensions < min_dimensions || dimensions > max_dimensions) {
                    throw std::invalid_argument("Dimension outside of range [" + 
                        std::to_string(min_dimensions) + ", " + std::to_string(max_dimensions) + "]");
                }
            } catch (std::invalid_argument& e) {
                print_usage();
                exit(-1);
            }
        }
    }

    while (player_index < 2) {
        players[player_index++] = new HumanPlayer();
    }

    Game game(*players[0], *players[1], dimensions);
    GameResult res;

    do {
        game.Display();
        res = game.ExecutePly();
    } while (res == GameResult::Ongoing);

    game.Display();
    std::cout << res << std::endl;

    return 0;
}