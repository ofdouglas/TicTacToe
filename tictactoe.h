#include <iostream>

namespace TicTacToe {

    enum class GameResult { Ongoing, Draw, X_win, O_win };
    enum class Mark { Empty, X, O };

    static constexpr int num_rows = 3;
    static constexpr int num_cols = 3;
    static constexpr int num_squares = num_rows * num_cols;

    class Board;

    struct Move {
        int row;
        int col;
    };

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
        Move GetMove(const Board& b, Mark mark) const;

    private:
        int Negamax(Board& b, Mark mark, int depth) const;
        mutable Move best_move;
    };

    class Board {
    public:
        Board();
        void ApplyMove(Move move, Mark sc);
        void UndoMove(Move move);
        bool IsValidMove(Move move) const;
        bool IsInBoundsMove(Move move) const;
        GameResult CheckResults() const;

    private:
        Mark squares[num_rows][num_cols] = {};

        bool HasWon(Mark mark) const;
        friend std::ostream& operator<<(std::ostream&, const Board& b);
    };

    class Game {
    public:
        Game(Player& x_player, Player& o_player);
        GameResult ExecutePly();
        void Display() const;

    private:
        Board board;
        Player* const players[2];
        unsigned ply_number;
    };
};
