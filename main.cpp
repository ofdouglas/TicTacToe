#include <iostream>
#include <cassert>
#include <limits>
#include <algorithm>

#include "tictactoe.h"

namespace TicTacToe {

    /**************************************************************************
     * Text output
     *************************************************************************/
    std::ostream& operator<<(std::ostream& os, TicTacToe::GameResult result) {
        switch (result) {
            case TicTacToe::GameResult::Draw:   return os << "Draw";    break;
            case TicTacToe::GameResult::X_win:  return os << "X wins";  break;
            case TicTacToe::GameResult::O_win:  return os << "O wins";  break;
        }
        return os << static_cast<char>(result);
    }

    std::ostream& operator<<(std::ostream& os, TicTacToe::Mark mark) {
        switch (mark) {
            case TicTacToe::Mark::Empty:    return os << " ";   break;
            case TicTacToe::Mark::X:        return os << "X";   break;
            case TicTacToe::Mark::O:        return os << "O";   break;
        }
        return os << static_cast<char>(mark);
    }

    std::ostream& operator<<(std::ostream& os, const TicTacToe::Board& board) {
        for (int i = board.dimension - 1; i >= 0; i--) {
            os << i << " ";
            for (int j = 0; j < board.dimension; j++) {
                os << "| " << board.At(i,j) << " ";
            }
            os << "|" << std::endl;
        }
        os << "   ";
        for (int i = 0; i < board.dimension; i++) {
            os << " " << i << "  ";
        }
        os << std::endl;

        return os;
    }


    /**************************************************************************
     * Board class
     *************************************************************************/
    Board::Board(int dim) : dimension(dim) {
        squares = new Mark[dimension * dimension];

        for (int i = 0; i < dimension * dimension; i++) {
            squares[i] = Mark::Empty;
        }
    }

    Mark& Board::At(int row, int col) {
        return squares[row + col * dimension];
    }

    const Mark& Board::At(int row, int col) const {
        return squares[row + col * dimension];
    }

    void Board::ApplyMove(Move move, Mark mark) {
        assert(IsValidMove(move));
        At(move.row, move.col) = mark;
    }

    void Board::UndoMove(Move move) {
        assert(IsInBoundsMove(move));
        assert(At(move.row, move.col) != Mark::Empty);
        At(move.row, move.col) = Mark::Empty;
    }

    bool Board::IsInBoundsMove(Move move) const {
        return move.row >= 0 && move.row < dimension && move.col >= 0 && move.col < dimension;
    }

    bool Board::IsValidMove(Move move) const {
        return IsInBoundsMove(move) && At(move.row, move.col) == Mark::Empty;
    }

    bool Board::HasWon(Mark mark) const {
        int row, col;

        // Check for column-based victory
        for (row = 0; row < dimension; row++) {
            for (col = 0; col < dimension; col++) {
                if (At(row, col) != mark) {
                    break;
                }
            }
            if (col == dimension) {
                return true;
            }
        }

        // Check for row-based victory
        for (col = 0; col < dimension; col++) {
            for (row = 0; row < dimension; row++) {
                if (At(row, col) != mark) {
                    break;
                }
            }
            if (row == dimension) {
                return true;
            }
        }

        // Check for diagonal-based victory
        for (row = 0; row < dimension; row++) {
            if (At(row, col) != mark) {
                break;
            }
        }
        if (row == dimension) {
            return true;
        }

        // Check for anti-diagonal-based victory
        for (row = 0; row < dimension; row++) {
            if (At(row, dimension - row - 1) != mark) {
                break;
            }
        }
        if (row == dimension) {
            return true;
        }

        return false;
    }

    GameResult Board::CheckResults() const {
        if (HasWon(Mark::X)) {
            return GameResult::X_win;
        } else if (HasWon(Mark::O)) {
            return GameResult::O_win;
        }

        for (int i = 0; i < dimension * dimension; i++) {
            if (squares[i] == Mark::Empty) {
                return GameResult::Ongoing;
            }
        }

        return GameResult::Draw;
    }


    /**************************************************************************
     * Game class
     *************************************************************************/
    Game::Game(Player& x_player, Player& o_player, int dimension) 
        : board{dimension}, players{&x_player, &o_player}, ply_number{0}
        {}

    GameResult Game::ExecutePly() {
        unsigned player_index = ply_number % 2;
        Mark mark = player_index ? Mark::X : Mark::O;

        std::cout << "Player " << mark << " to move: " << std::endl;

        Move move = players[player_index]->GetMove(board, mark);
        board.ApplyMove(move, mark);

        GameResult res = board.CheckResults();
        ply_number++;
        return res;
    }

    void Game::Display() const {
        std::cout << board;
    }


    /**************************************************************************
     * HumanPlayer class
     *************************************************************************/
    int HumanPlayer::ReadIntWithPrompt(const std::string& prompt) const {
        int x;

        while (true) {
            std::cout << prompt;
            if (!(std::cin >> x)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } else {
                break;
            }
        }

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return x;
    }

    Move HumanPlayer::GetMove(const TicTacToe::Board& board, Mark mark) const {
        Move move;

        while (true) {
            move.row = ReadIntWithPrompt("Row: ");
            move.col = ReadIntWithPrompt("Col: ");

            if (board.IsValidMove(move)) {
                break;
            } else {
                std::cout << "Invalid move" << std::endl;
            }
        }

        return move;
    }


    /**************************************************************************
     * ComputerPlayer class
     *************************************************************************/
    int ComputerPlayer::Negamax(TicTacToe::Board& board, Mark mark, int depth) const {
        GameResult res = board.CheckResults();
        if (res == GameResult::Draw) {
            return 0;
        }
        if (res == GameResult::X_win) {
            return mark == Mark::X ? +1 : -1;
        }
        if (res == GameResult::O_win) {
            return mark == Mark::X ? -1 : +1;
        }

        int value = std::numeric_limits<int>::min();

        // Generate and try all legal moves
        for (int i = 0; i < board.dimension; i++) {
            for (int j = 0; j < board.dimension; j++) {
                Move move = {i,j};
                
                if (board.IsValidMove(move)) {
                    board.ApplyMove(move, mark);
                    Mark new_mark = mark == Mark::X ? Mark::O : Mark::X;

                    int new_value = -Negamax(board, new_mark, depth + 1);
                    if (new_value > value) {
                        value = new_value;
                        if (depth == 0) {
                            best_move = move;
                        }
                    }

                    board.UndoMove(move);
                }
            }
        }

        return value;
    }

    Move ComputerPlayer::GetMove(const TicTacToe::Board& b, Mark mark) const {
        TicTacToe::Board bb = b;
        Negamax(bb, mark, 0);
        return best_move;
    }
}

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