#include "test.h"

#include <cassert>

#include "solver.h"
#include "printer.h"
#include "MoveDiscovery.h"
#include "MoveValidation.h"


namespace
{
    // Standard klotski puzzle
    // print for sample ascii layout
    const Puzzle initialBoard
    {
        { 4, 6 }, // dims
        { 1, 4, 2, 2, " "}, // goal
        { // invalid spaces
            { 0, 5 },
            { 3, 5 },
        },
        { // Initial board state
            0, // no moves made,
            { 1, 0, 2, 2, "@"}, // runner
            { // blocks
                { 0, 0, 1, 2, "A"},
                { 0, 2, 1, 2, "B" },
                { 1, 2, 2, 1, "C" },
                { 1, 3, 1, 1, "D" },
                { 2, 3, 1, 1, "E" },
                { 3, 0, 1, 2, "F" },
                { 3, 2, 1, 2, "G" },
                { 0, 4, 1, 1, "H" },
                { 3, 4, 1, 1, "I" }
            }
        }
    };

    // Empty 3x3 block
    // Runner 1x1 at the origin
    // Single 1x1 block in the middle of the board
    const Puzzle emptyPuzzle
    {
        { 3, 3 }, // dims
        { 2, 2, 1, 1, " " }, // goal
        {}, // empty spaces
        {
            0, // no moves made,
            { 0, 0, 1, 1, "@"}, // runner at origin
            { // single hindrance in the middle
                { 1, 1 }
            }
        }
    };

    const Puzzle tinyPuzzle
    {
        { 2, 2 }, // dims
        { 1, 1, 1, 1, " "}, // goal
        {}, // empty spaces
        {
            0, // no moves made,
            { 0, 0, 1, 1, "@"}, // runner in the middle
            {} // no other blocks
        }
    };

    // Empty 3x3 block
    // Runner 1x1 at the origin
    // Single 1x1 block in the middle of the board
    const Puzzle smallPuzzle
    {
        { 3, 3 }, // dims
        { 2, 2, 1, 1, " " }, // goal
        {
            { 1, 1 }
        }, // empty spaces
        {
            0, // no moves made,
            { 0, 0, 1, 1, "@"}, // runner at origin
            { // 2 blocks surrounding origin
                { 1, 0, 1, 1, "A"},
                { 0, 1, 1, 1, "B" }
            }
        }
    };
}

void testPuzzles()
{
    assert(valid(emptyPuzzle));
    assert(valid(tinyPuzzle));
    assert(valid(smallPuzzle));
    assert(valid(initialBoard));
}

void testBlocks()
{
    Block singleLeft{ 1, 1, 1, 1 };
    Block singleRight{ 3, 1, 1, 1 };
    Block singleMiddle{ 2, 1, 1, 1 };
    Block doubleLeft{ 0, 0, 2, 1 };
    Block doubleRight{ 1, 0, 2, 1 };
    Block somewhereElse{ 2, 2, 1, 1 };

    assert(!overlaps(singleLeft, singleRight));
    assert(!overlaps(singleLeft, singleMiddle));
    assert(overlaps(doubleLeft, doubleRight) && "Block should overlap in the middle");
    assert(overlaps(doubleRight, doubleLeft) && "Block should overlap in the middle");
    assert(!overlaps(doubleLeft, somewhereElse) && "Blocks should not overlap");
    assert(!overlaps(doubleRight, somewhereElse) && "Blocks should not overlap");

    Block middle{ 2, 2, 2, 2 };
    Block origin{ 0, 0, 1, 1 };
    std::vector<Point> freeSpaceLeft{ { 1, 2 } };
    std::vector<Point> freeSpaceRight{ { 4, 2 } };
    std::vector<Point> freeSpaceTop{ { 3, 1 } };
    std::vector<Point> freeSpaceBottom{ { 3, 4} };

    assert(nextToFreeSpace(middle, freeSpaceLeft) && "Block should be next to free space");
    assert(nextToFreeSpace(middle, freeSpaceRight) && "Block should be next to free space");
    assert(nextToFreeSpace(middle, freeSpaceTop) && "Block should be next to free space");
    assert(nextToFreeSpace(middle, freeSpaceBottom) && "Block should be next to free space");
    assert(!nextToFreeSpace(origin, freeSpaceBottom) && "Origin should not be next to free space");
    assert(!nextToFreeSpace(origin, freeSpaceTop) && "Origin should not be next to free space");
    assert(!nextToFreeSpace(origin, freeSpaceRight) && "Origin should not be next to free space");
    assert(!nextToFreeSpace(origin, freeSpaceLeft) && "Origin should not be next to free space");

    assert(equalPosition(move(middle, Direction::Up), Block{ 2, 1, 2, 2 }));
    assert(equalPosition(move(middle, Direction::Down), Block{ 2, 3, 2, 2 }));
    assert(equalPosition(move(middle, Direction::Left), Block{ 1, 2, 2, 2 }));
    assert(equalPosition(move(middle, Direction::Right), Block{ 3, 2, 2, 2 }));

    assert(same(move(middle, Direction::Up), Block{ 2, 1, 2, 2, middle.id }));
    assert(same(move(middle, Direction::Down), Block{ 2, 3, 2, 2, middle.id }));
    assert(same(move(middle, Direction::Left), Block{ 1, 2, 2, 2, middle.id }));
    assert(same(move(middle, Direction::Right), Block{ 3, 2, 2, 2, middle.id }));
}

void testMoveValidation()
{
    // Test if class can be constructed
    DefaultMoveValidation moveValidation{};

    // Test that all blocks do not overlap with their current state on the board
    for (const auto &block : initialBoard.m_initialState.m_blocks)
    {
        assert(
            DefaultMoveValidation::validBlockPosition(block, initialBoard.m_dimensions, initialBoard.m_initialState, initialBoard.m_forbiddenSpots)
            && "A block should never overlap itself");

        auto blockAtSamePosition = block;
        blockAtSamePosition.id = "Another Block";
        assert(
            !DefaultMoveValidation::validBlockPosition(blockAtSamePosition, initialBoard.m_dimensions, initialBoard.m_initialState, initialBoard.m_forbiddenSpots)
            && "2 blocks at the same position should always overlap");
    }

    // Test that a block can move in all directions on an empty board
    const auto isRunnerPosition = [&](const auto x, const auto y)
    {
        return x == emptyPuzzle.m_initialState.m_runner.m_startX
            && y == emptyPuzzle.m_initialState.m_runner.m_startY;
    };

    const auto isBlockPosition = [&](const auto x, const auto y)
    {
        return x == emptyPuzzle.m_initialState.m_blocks[0].m_startX
            && y == emptyPuzzle.m_initialState.m_blocks[0].m_startY;
    };

    for (auto i = 0; i < emptyPuzzle.m_dimensions.m_x; i++)
    {
        for (auto j = 0; j < emptyPuzzle.m_dimensions.m_y; ++j)
        {
            if (!isRunnerPosition(i, j) && !isBlockPosition(i, j)) 
            {
                const auto block = Block{ i, j, 1, 1, "someBlock" };
                assert(
                    DefaultMoveValidation::validBlockPosition(block, emptyPuzzle.m_dimensions, emptyPuzzle.m_initialState, emptyPuzzle.m_forbiddenSpots)
                    && "Block should be able to be positioned anywhere except origin & center");
            }
        }
    }
}

void testMoveDiscovery()
{
    // Test if class can be constructed
    MoveRunnerFirst<> moveRunnerFirst{};

    {
        const auto runnerMoves = 2; // Down + Right
        const auto blockMoves = 4; // Up + Down + Right + Left
        const auto newMoves = MoveRunnerFirst<>::gatherMoves(
            emptyPuzzle.m_dimensions,
            std::make_shared<BoardState>(emptyPuzzle.m_initialState),
            emptyPuzzle.m_forbiddenSpots);
        assert(!newMoves.empty() && "At least some moves should be possible");
        assert(newMoves.size() == (runnerMoves + blockMoves) && "We should have 6 moves discovered");
    }
    
    {
        const auto blockMoves = 4;
        auto newMoves = MoveRunnerFirst<>::gatherMoves(
            initialBoard.m_dimensions,
            std::make_shared<BoardState>(initialBoard.m_initialState),
            initialBoard.m_forbiddenSpots);
        assert(!newMoves.empty() && "At least some moves should be possible");
        assert(newMoves.size() == blockMoves && "We should have 4 moves discovered");
    }
}

void testMoving()
{
    const auto sharedState = std::make_shared<BoardState>(initialBoard.m_initialState);
    Move moveRight{ sharedState, sharedState->m_blocks[0], Direction::Right };
    assert(sharedState.get() == moveRight.m_state.get() && "A move should not copy the state, but only reference the original state");

    const auto afterMoveRight = moveRight();
    assert(equalPosition(move(sharedState->m_blocks[0], Direction::Right), afterMoveRight.m_blocks[0]));
    assert(!equalPosition(move(sharedState->m_blocks[0], Direction::Left), afterMoveRight.m_blocks[0]));
    assert(!equalPosition(move(sharedState->m_blocks[0], Direction::Up), afterMoveRight.m_blocks[0]));
    assert(!equalPosition(move(sharedState->m_blocks[0], Direction::Down), afterMoveRight.m_blocks[0]));
    assert(same(sharedState->m_blocks[0], afterMoveRight.m_blocks[0]));

    assert(sharedState->m_numberOfMovesFromStart + 1 == afterMoveRight.m_numberOfMovesFromStart);
}

void testHashing()
{
    // Test if type can be constructed
    BoardHasher<> hasher{};

    assert(BoardHasher<>::hash(emptyPuzzle.m_initialState) == BoardHasher<>::hash(emptyPuzzle.m_initialState)
        && "Hash for identical states should be equal");
    assert(BoardHasher<>::hash(initialBoard.m_initialState) == BoardHasher<>::hash(initialBoard.m_initialState)
        && "Hash for identical states should be equal");
    assert(BoardHasher<>::hash(initialBoard.m_initialState) != BoardHasher<>::hash(emptyPuzzle.m_initialState)
        && "Hash for different states should be different");
}

void testSolver()
{
    {
        Solver<MoveRunnerFirst> solver{ tinyPuzzle };
        assert(
            !solver.possibleMoves().empty()
            && "A few initial moves are available");

        const auto result = solver.solve();
        assert(result < 4 && result > 0 && "Tiny puzzle should be solved in less than 4 moves");
    }

    {
        Solver<MoveRunnerFirst> solver{ emptyPuzzle };
        assert(
            !solver.possibleMoves().empty()
            && "A few initial moves are available");

        const auto result = solver.solve();
        assert(result < 6 && result > 0 && "Empty puzzle should be solved in less than 6 moves");
    }

    {
        Solver<MoveRunnerFirst> solver{ smallPuzzle };
        assert(
            !solver.possibleMoves().empty()
            && "A few initial moves are available");

        const auto result = solver.solve();
        assert(result < 10 && result > 0 && "Small puzzle should be solved in less than 10 moves");
    }

    {
        Solver<MoveRunnerFirst> solver{ initialBoard };
        assert(
            !solver.possibleMoves().empty()
            && "A few initial moves are available");

        const auto result = solver.solve();
        std::cout << "##### Result is" << result << std::endl;
        assert(result > 0 && result < 200 && "Not sure what a reasonable number of moves is here...");
    }
}

int main(int argc, char *argv[])
{
    testBlocks();
    testPuzzles();
    testMoveValidation();
    testMoveDiscovery();
    testMoving();
    testHashing();
    testSolver();
}