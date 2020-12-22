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

    std::ostream& operator<<(std::ostream& os, const TicTacToe::Board& b) {
        for (int i = TicTacToe::num_rows - 1; i >= 0; i--) {
            os << i << " ";
            for (int j = 0; j < TicTacToe::num_cols; j++) {
                os << "| " << b.squares[i][j] << " ";
            }
            os << "|" << std::endl;
        }
        os << "    0   1   2\n" << std::endl;

        return os;
    }


    /**************************************************************************
     * Board class
     *************************************************************************/
    Board::Board() {
        for (auto& col : squares) {
            for (auto& sq : col) {
                sq = Mark::Empty;
            }
        }
    }

    void Board::ApplyMove(Move move, Mark mark) {
        assert(IsValidMove(move));
        squares[move.row][move.col] = mark;
    }

    void Board::UndoMove(Move move) {
        assert(IsInBoundsMove(move));
        assert(squares[move.row][move.col] != Mark::Empty);
        squares[move.row][move.col] = Mark::Empty;
    }

    bool Board::IsInBoundsMove(Move move) const {
        return move.row >= 0 && move.row < num_rows && move.col >= 0 && move.col < num_cols;
    }

    bool Board::IsValidMove(Move move) const {
        return IsInBoundsMove(move) && squares[move.row][move.col] == Mark::Empty;
    }

    bool Board::HasWon(Mark mark) const {
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
        if (HasWon(Mark::X)) {
            return GameResult::X_win;
        } else if (HasWon(Mark::O)) {
            return GameResult::O_win;
        }

        // A quicker way to detect a draw: ply_number == num_squares
        for (auto& col : squares) {
            for (auto& s : col) {
                if (s == Mark::Empty) {
                    return GameResult::Ongoing;
                }
            }
        }

        return GameResult::Draw;
    }


    /**************************************************************************
     * Game class
     *************************************************************************/
    Game::Game(Player& x_player, Player& o_player) 
        : players{ &x_player, &o_player }, ply_number(0)
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
        for (int i = 0; i < num_rows; i++) {
            for (int j = 0; j < num_cols; j++) {
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
        //Negamax(TicTacToe::Board(b), mark, 0); -- this does not work. Why?
        TicTacToe::Board bb = b;
        Negamax(bb, mark, 0);
        return best_move;
    }
}


int main(void) {
    using namespace TicTacToe;

    HumanPlayer p1;
    ComputerPlayer p2;
    Game game(p1, p2);

    GameResult res;
    do {
        game.Display();
        res = game.ExecutePly();
    } while (res == GameResult::Ongoing);

    game.Display();
    std::cout << res << std::endl;

    return 0;
}