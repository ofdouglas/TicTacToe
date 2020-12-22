#include <iostream>

namespace TicTacToe {

    enum class GameResult { Ongoing, Draw, X_win, O_win };
    enum class SquareContents { Empty, X_owned, O_owned };

    static constexpr int num_rows = 3;
    static constexpr int num_cols = 3;
    static constexpr int num_squares = num_rows * num_cols;

    class Board;

    struct Move {
        bool IsInBounds();
        int row;
        int col;
    };

    class Player {
    public:
        virtual Move GetMove(const Board& b) = 0;
    };

    class HumanPlayer : public Player {
    public:
        HumanPlayer(SquareContents mark) { this->mark = mark; }
        Move GetMove(const Board& b);
    private:
        int ReadIntWithPrompt(const std::string& prompt);
        SquareContents mark;
    };

    class ComputerPlayer : public Player {
    public:
        ComputerPlayer(SquareContents mark) { this->mark = mark; }
        Move GetMove(const Board& b);
    private:
        int Negamax(const Board& b, SquareContents mark, int depth);
        SquareContents mark;
        Move best_move;
    };

    class Board {
    public:
        Board();

        // Throws if an invalid move is applied, so HumanPlayer should validate the move first
        // using IsValidMove
        void ApplyMove(Move move, SquareContents sc);
        
        bool IsValidMove(Move move) const;

        GameResult CheckResults() const;

    private:
        friend std::ostream& operator<<(std::ostream&, const TicTacToe::Board& b);
        SquareContents squares[num_rows][num_cols];

        bool HasWon(SquareContents mark) const;
    };

    class Game {
    public:
        Game(Player& x_player, Player& o_player);
        GameResult ExecutePly();
        void Display();

    private:
        Board board;
        Player* const players[2];
        unsigned ply_number;
    };
};



