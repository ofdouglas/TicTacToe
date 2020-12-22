#include <iostream>
#include <cassert>
#include <limits>
#include <algorithm>

#include "tictactoe.h"

/* Tic-tac-toe game components:
 
 * Board representation (internal)
 * UI (Board display and move input)
 * Computer move selection
 * Game rules (move legality, victory/draw conditions)
 * Other game state: whose turn is it?


Board class:
  - internal representation
  - game rules
  - board display
*/

namespace TicTacToe {

    bool Move::IsInBounds() {
        return row >= 0 && row < num_rows &&
            col >= 0 && col < num_cols;
    }

    Board::Board() {
        for (auto& col : squares) {
            for (auto& sq : col) {
                sq = SquareContents::Empty;
            }
        }
    }

    void Board::ApplyMove(Move move, SquareContents sc) {
        if (!move.IsInBounds()) {
            throw std::logic_error("Board::ApplyMove: out of bounds move");
        }

        if (squares[move.row][move.col] != SquareContents::Empty) {
            throw std::logic_error("Board::ApplyMove square already occupied");
        }

        squares[move.row][move.col] = sc;
    }

    bool Board::IsValidMove(Move move) const {
        if (!move.IsInBounds()) {
            return false;
        }
        return squares[move.row][move.col] == SquareContents::Empty;
    }

    bool Board::HasWon(SquareContents mark) const {
        for (int i = 0; i < TicTacToe::num_rows; i++) {
            if (squares[i][0] == mark && squares[i][1] == mark && squares[i][2] == mark) {
                return true;
            }
        }

        for (int i = 0; i < TicTacToe::num_rows; i++) {
            if (squares[0][i] == mark && squares[1][i] == mark && squares[2][i] == mark) {
                return true;
            }
        }

        if (squares[0][0] == mark && squares[1][1] == mark && squares[2][2] == mark) {
            return true;
        }

        if (squares[0][2] == mark && squares[1][1] == mark && squares[2][0] == mark) {
            return true;
        }

        return false;
    }

    GameResult Board::CheckResults() const {
        if (HasWon(SquareContents::X_owned)) {
            return GameResult::X_win;
        } else if (HasWon(SquareContents::O_owned)) {
            return GameResult::O_win;
        }

        // A quicker way to detect a draw: ply_number == num_squares
        for (auto& col : squares) {
            for (auto& s : col) {
                if (s == SquareContents::Empty) {
                    return GameResult::Ongoing;
                }
            }
        }

        return GameResult::Draw;
    }


    std::string str_from_square_contents(TicTacToe::SquareContents sc) {
        switch (sc) {
            case TicTacToe::SquareContents::Empty:     return " ";   break;
            case TicTacToe::SquareContents::X_owned:   return "X";   break;
            case TicTacToe::SquareContents::O_owned:   return "O";   break;
        }
        assert(0);
        return "";
    }

    std::ostream& operator<<(std::ostream& os, const TicTacToe::Board& b) {
        for (int i = TicTacToe::num_rows - 1; i >= 0; i--) {
            os << i << " ";
            for (int j = 0; j < TicTacToe::num_cols; j++) {
                os << "| " << str_from_square_contents(b.squares[i][j]) << " ";
            }
            os << "|" << std::endl;
        }
        os << "    0   1   2\n" << std::endl;

        return os;
    }

    Game::Game(Player& x_player, Player& o_player) 
        : players{ &x_player, &o_player }, ply_number(0)
        {}

    GameResult Game::ExecutePly() {
        unsigned player_index = ply_number % 2;
        SquareContents player_mark = player_index ? 
            SquareContents::X_owned : SquareContents::O_owned;

        Move move = players[player_index]->GetMove(board);
        board.ApplyMove(move, player_mark);

        GameResult res = board.CheckResults();
        ply_number++;
        return res;
    }

    void Game::Display() {
        std::cout << board;
    }

    int HumanPlayer::ReadIntWithPrompt(const std::string& prompt) {
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

    // TODO: implement me!
    Move HumanPlayer::GetMove(const TicTacToe::Board& board) {
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


    int ComputerPlayer::Negamax(const TicTacToe::Board& b, SquareContents mark, int depth) {
        GameResult res = b.CheckResults();
        if (res == GameResult::Draw) {
            return 0;
        }
        if (res == GameResult::X_win) {
            return mark == SquareContents::X_owned ? +1 : -1;
        }
        if (res == GameResult::O_win) {
            return mark == SquareContents::X_owned ? -1 : +1;
        }

        int value = -1000;

        // Generate and try all legal moves
        for (int i = 0; i < num_rows; i++) {
            for (int j = 0; j < num_cols; j++) {
                Move move = {i,j};
                
                if (b.IsValidMove(move)) {
                    TicTacToe::Board new_board = b;
                    SquareContents new_mark = mark == SquareContents::X_owned ? SquareContents::O_owned : SquareContents::X_owned;
                    new_board.ApplyMove(move, mark);

                    int new_value = -Negamax(new_board, new_mark, depth + 1);
                    if (new_value > value) {
                        value = new_value;
                        if (depth == 0) {
                            best_move = move;
                        }
                    }
                }
            }
        }

        return value;
    }

    // TODO: implement me!
    Move ComputerPlayer::GetMove(const TicTacToe::Board& b) {
        Negamax(b, mark, 0);
        return best_move;
    }
}

int main(void) {
    // TODO: this allows us to choose if X or O goes first... Game class assumes O goes first!
    TicTacToe::HumanPlayer p1(TicTacToe::SquareContents::O_owned);
    //TicTacToe::ComputerPlayer p1(TicTacToe::SquareContents::O_owned);
    TicTacToe::ComputerPlayer p2(TicTacToe::SquareContents::X_owned);
    TicTacToe::Game game(p1, p2);

    TicTacToe::GameResult res;
    do {
        game.Display();
        res = game.ExecutePly();
    } while (res == TicTacToe::GameResult::Ongoing);

    game.Display();
    switch (res) {
        case TicTacToe::GameResult::Draw:   std::cout << "Draw"   << std::endl;   break;
        case TicTacToe::GameResult::X_win:  std::cout << "X wins" << std::endl;   break;
        case TicTacToe::GameResult::O_win:  std::cout << "O wins" << std::endl;   break;       
    }

    return 0;
}