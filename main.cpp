#include "solver.h"

int main(int argc, char *argv[])
{
    // Standard klotski puzzle
    // print for sample ascii layout
    const Puzzle<9> largePuzzle
    {
        { 4, 6 }, // dims
        { 1, 4, 2, 2, "^" }, // goal
        { // invalid spaces
            { 0, 5 },
            { 3, 5 },
        },
        { // Initial board state
            0, // no moves made,
            { 1, 0, 2, 2, "@" }, // runner
            { // blocks
                Block{ 0, 0, 1, 2, "A" },
                Block{ 0, 2, 1, 2, "B" },
                Block{ 1, 2, 2, 1, "C" },
                Block{ 1, 3, 1, 1, "D" },
                Block{ 2, 3, 1, 1, "E" },
                Block{ 3, 0, 1, 2, "F" },
                Block{ 3, 2, 1, 2, "G" },
                Block{ 0, 4, 1, 1, "H" },
                Block{ 3, 4, 1, 1, "I" }
            }
        }
    };

    auto solver = makeSolver(largePuzzle);

    const auto result = solver.solve();
    if (result < 0)
    {
        std::cout << "Could not find a solution." << std::endl;
        return 1;
    }
    else
    {
        std::cout << result << std::endl;
        return 0;
    }
}
