#include <iostream>
#include <stdexcept>
#include <cassert>
#include <ctime>
#include "tictactoe.h"

namespace TicTacToe {

    using std::string;
    using std::cin;
    using std::cout;
    using std::endl;

    /**************************************************************************
     * Debug Options
     *************************************************************************/    
    enum class DebugOptions { 
        HeuristicScore,
        EvaluationBeforeMove,
        MoveSelectionStats,
        BestMove,
        Negamax
    };

    enum FlagBits {
        HeuristicScore = (1UL << static_cast<int>(DebugOptions::HeuristicScore)),
        EvaluationBeforeMove = (1UL << static_cast<int>(DebugOptions::EvaluationBeforeMove)),
        MoveSelectionStats = (1UL << static_cast<int>(DebugOptions::MoveSelectionStats)),
        BestMove = (1UL << static_cast<int>(DebugOptions::BestMove)),
        Negamax = (1UL << static_cast<int>(DebugOptions::Negamax))
    };

    static constexpr uint32_t debug_flags = 0;


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

    string Move::ToString() const {
        return string(
            "[" + std::to_string(row) + ", " + std::to_string(col) + "]");
    }

    std::ostream& operator<<(std::ostream& os, const TicTacToe::Board& board) {
        for (int i = board.dimension - 1; i >= 0; i--) {
            os << i << " ";
            for (int j = 0; j < board.dimension; j++) {
                os << "| " << board.At(i,j) << " ";
            }
            os << "|" << endl;
        }
        os << "   ";
        for (int i = 0; i < board.dimension; i++) {
            os << " " << i << "  ";
        }
        os << endl << endl;

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
        if (!IsValidMove(move)) {
            throw std::invalid_argument(
                string(__func__).append(": invalid move: ").append(move.ToString()));
        }

        At(move.row, move.col) = mark;
    }

    void Board::UndoMove(Move move) {
        if (!IsInBoundsMove(move)) {
            throw std::invalid_argument(
                string(__func__).append(": out of bounds move: ").append(move.ToString()));
        }
        if (At(move.row, move.col) == Mark::Empty) {
            throw std::invalid_argument(
                string(__func__).append(": undo null move: ").append(move.ToString()));
        }

        At(move.row, move.col) = Mark::Empty;
    }

    bool Board::IsInBoundsMove(Move move) const {
        return move.row >= 0 && move.row < dimension && move.col >= 0 && move.col < dimension;
    }

    bool Board::IsValidMove(Move move) const {
        return IsInBoundsMove(move) && At(move.row, move.col) == Mark::Empty;
    }

    score_t Board::ScoreLine(const std::vector<Mark>& line) const {
        score_t x_count = 0;
        score_t o_count = 0;

        for (int i = 0; i < dimension; i++) {
            x_count += (line[i] == Mark::X) ? 1 : 0;
            o_count += (line[i] == Mark::O) ? 1 : 0;
        }
        if (x_count > 0 && o_count > 0) {
            return 0;
        } else {
            score_t diff = x_count - o_count;
            if (std::abs(diff) == dimension) {
                return (diff > 0) ? +max_score : -max_score;
            } else {
                return diff * score_per_mark;
            }
        }
    }

    score_t Board::HeuristicScore() const {
        score_t score = 0;
        score_t new_score;

        // Add score from each row
        for (int row = 0; row < dimension; row++) {
            for (int col = 0; col < dimension; col++) {
                line[col] = At(row, col);
            }
            new_score = ScoreLine(line);
            score += new_score;
            if constexpr(debug_flags & FlagBits::HeuristicScore) {
                cout << "Row[" << row << "] score = " << new_score << endl;
            }
        }

        // Add score from each column
        for (int col = 0; col < dimension; col++) {
            for (int row = 0; row < dimension; row++) {
                line[row] = At(row, col);
            }
            new_score = ScoreLine(line);
            score += new_score;
            if constexpr(debug_flags & FlagBits::HeuristicScore) {
                cout << "Col[" << col << "] score = " << new_score << endl;
            }
        }

        // Add score from the diagonal
        for (int i = 0; i < dimension; i++) {
            line[i] = At(i, i);
        }
        new_score = ScoreLine(line);
        score += new_score;
        if constexpr(debug_flags & FlagBits::HeuristicScore) {
            cout << "Diagonal score = " << new_score << endl;
        }

        // Add score from the anti-diagonal
        for (int i = 0; i < dimension; i++) {
            line[i] = At(dimension - i - 1, i);
        }
        new_score = ScoreLine(line);
        score += new_score;
        if constexpr(debug_flags & FlagBits::HeuristicScore) {
            cout << "Anti-diagonal score = " << new_score << endl;
        }

        return score;
    }

    bool Board::IsAnyTileEmpty() const {
        for (int i = 0; i < dimension * dimension; i++) {
            if (squares[i] == Mark::Empty) {
                return true;
            }
        }
        return false;
    }

    GameResult Board::CheckResults() const {
        score_t score = HeuristicScore();

        if (score >= max_score) {
            return GameResult::X_win;
        } else if (score <= -max_score) {
            return GameResult::O_win;
        } else if (IsAnyTileEmpty()) {
            return GameResult::Ongoing;
        } else {
            return GameResult::Draw;            
        }
    }


    /**************************************************************************
     * Game class
     *************************************************************************/
    Game::Game(Player& x_player, Player& o_player, int dimension) 
        : board{dimension}, players{&x_player, &o_player}, ply_number{0}
        {}

    GameResult Game::ExecutePly() {
        unsigned player_index = ply_number % 2;
        Mark mark = player_index ? Mark::O : Mark::X;

        cout << "Player " << mark << " to move: " << endl;

        if constexpr(debug_flags & FlagBits::EvaluationBeforeMove) {
            cout << "Evaluation before move: " << board.HeuristicScore() << endl;
        }

        Move move = players[player_index]->GetMove(board, mark);
        board.ApplyMove(move, mark);

        GameResult res = board.CheckResults();
        ply_number++;
        return res;
    }

    void Game::Display() const {
        cout << board;
    }


    /**************************************************************************
     * HumanPlayer class
     *************************************************************************/
    int HumanPlayer::ReadIntWithPrompt(const string& prompt) const {
        int x;

        while (true) {
            cout << prompt;
            if (!(cin >> x)) {
                cin.clear();
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } else {
                break;
            }
        }

        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
                cout << "Invalid move" << endl;
            }
        }

        return move;
    }


    /**************************************************************************
     * ComputerPlayer class
     *
     * TODO: Add pruning
     *************************************************************************/
    score_t ComputerPlayer::Negamax(TicTacToe::Board& board, Mark player, unsigned depth) const {
        score_t current_score = board.HeuristicScore() * player;

        if constexpr(debug_flags & MoveSelectionStats ) {
            moves_evaluated++;
            if (depth > max_depth_this_run) {
                max_depth_this_run = depth;
            }
        }

        // Return at depth limit, a win for either side, or a draw.
        if (depth == depth_limit || std::abs(current_score) >= max_score || !board.IsAnyTileEmpty()) {
            if constexpr(debug_flags & FlagBits::Negamax) {
                cout << "Returning score " << current_score << " from depth " << depth << endl;
            }
            return current_score;
        }

        score_t best_score = -max_score;

        // Generate and try all legal moves
        for (int i = 0; i < board.dimension; i++) {
            for (int j = 0; j < board.dimension; j++) {
                Move move = {i,j};
                
                if (board.IsValidMove(move)) {
                    board.ApplyMove(move, player);
                    Mark other_player = static_cast<Mark>(-static_cast<int>(player));
                    score_t new_score = -Negamax(board, other_player, depth + 1);

                    if (new_score > best_score) {
                        best_score = new_score;
                        if (depth == 1) {
                            move_selected = true;
                            best_move = move;

                            if constexpr(debug_flags & FlagBits::BestMove) {
                                cout << "Best move: " << move.ToString() << 
                                    ", score: " << best_score << endl;
                            }
                        }
                    }

                    board.UndoMove(move);
                }
            }
        }

        return best_score;
    }

    Move ComputerPlayer::GetMove(const TicTacToe::Board& b, Mark mark) const {
        move_selected = false;
        moves_evaluated = 0;
        max_depth_this_run = 0;
        clock_t start = clock();

        TicTacToe::Board bb = b;
        Negamax(bb, mark, 1);

        float time_diff_s = static_cast<float>(clock() - start) / CLOCKS_PER_SEC;

        if constexpr(debug_flags & FlagBits::MoveSelectionStats) {
            cout << "Selected move in " << time_diff_s << " s" << " [" << moves_evaluated 
                << " moves, depth = " << max_depth_this_run << "]" << endl;
        }
        assert(move_selected);
        return best_move;
    }
}