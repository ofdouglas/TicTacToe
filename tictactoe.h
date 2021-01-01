#ifndef TICTACTOE_H_DEFINED
#define TICTACTOE_H_DEFINED

#include <limits>
#include <cstdint>
#include <vector>
#include <string>
#include <cmath>

namespace TicTacToe {

    enum class GameResult { Ongoing, Draw, X_win, O_win };
    enum Mark { X = 1, Empty = 0, O = -1 };

    class Board;

    struct Move {
        int row;
        int col;

        std::string ToString() const;
    };

    // Use float so that score arithmetic saturates and latches at +/-INFINITY, as
    // required by the heuristic function.
    using score_t = float;
    static constexpr score_t max_score = INFINITY;
    static constexpr score_t score_per_mark = 1.0f;
    static constexpr size_t min_dimensions = 3;
    static constexpr size_t max_dimensions = 10;

    std::ostream& operator<<(std::ostream& os, TicTacToe::GameResult result);
    std::ostream& operator<<(std::ostream& os, TicTacToe::Mark mark);
    std::ostream& operator<<(std::ostream& os, TicTacToe::Move move);


    class Player {
    public:
        virtual Move GetMove(const Board& b, Mark mark) const = 0;
    };

    class HumanPlayer : public Player {
    public:
        Move GetMove(const Board& b, Mark mark) const;

    private:
        int ReadIntWithPrompt(const std::string& prompt) const;
    };

    class ComputerPlayer : public Player {
    public:
        ComputerPlayer(unsigned max_depth = 5) : depth_limit(max_depth) {}
        Move GetMove(const Board& b, Mark mark) const;

    private:
        score_t Negamax(Board& b, Mark mark, unsigned depth) const;

        mutable Move best_move;
        mutable bool move_selected;
        mutable unsigned moves_evaluated;
        mutable unsigned max_depth_this_run;

        const unsigned depth_limit;
    };

    class Board {
    public:
        Board(int dimension);

        bool IsValidMove(Move move) const;
        bool IsInBoundsMove(Move move) const;
        bool IsAnyTileEmpty() const;
        GameResult CheckResults() const;

        void ApplyMove(Move move, Mark sc);
        void UndoMove(Move move);

        // Requires that score_t::operator+= saturates and latches at max_score (the victory condition),
        // because the total score of a position is the sum of the scores of all lines, and we need any
        // max_score results from the line scoring function to be preserved, so that a winning line causes
        // the position to be scored as a win, even if the opponent has a better score for the other lines.
        // NOTE: addition of (+max_score) and (-max_score) should never happen (we do win checks each ply).
        score_t HeuristicScore() const;


        const int dimension;

    private:
        Mark* squares;              // Treated as a 2D array; accessed through 'At' function
        Mark& At(int row, int col);
        const Mark& At(int row, int col) const;

        // Calculate the score of a single row, column, or diagonal (e.g. a 'line'). The score is
        // zero if both players have marks on the line, because neither can win via that line. A win
        // is evaluated as a +max_score or -max_score, depending on the player. If the line isn't
        // contested and isn't a victory, the score is proportional to the number of marks. 
        inline score_t ScoreLine(const std::vector<Mark>& line) const;
        mutable std::vector<Mark> line = std::vector<Mark>(dimension);

        friend std::ostream& operator<<(std::ostream&, const Board& b);
    };

    class Game {
    public:
        Game(Player& x_player, Player& o_player, int dimension = 3);
        GameResult ExecutePly();
        void Display() const;

    private:
        Board board;
        Player* const players[2];
        unsigned ply_number;
    };
};

#endif // TICTACTOE_H_DEFINED