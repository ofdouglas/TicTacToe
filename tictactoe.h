
namespace TicTacToe {

    enum class GameResult { Ongoing, Draw, X_win, O_win };
    enum class Mark { Empty, X, O };

    std::ostream& operator<<(std::ostream& os, TicTacToe::GameResult result);
    std::ostream& operator<<(std::ostream& os, TicTacToe::Mark mark);

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
        Board(int dimension);
        void ApplyMove(Move move, Mark sc);
        void UndoMove(Move move);
        bool IsValidMove(Move move) const;
        bool IsInBoundsMove(Move move) const;
        GameResult CheckResults() const;

        const int dimension;

    private:
        Mark* squares;              // Treated as a 2D array; accessed through 'At' function
        Mark& At(int row, int col);
        const Mark& At(int row, int col) const;


        bool HasWon(Mark mark) const;
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
