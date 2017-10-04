#include "MoveDiscovery.h"

namespace
{
    constexpr auto numberOfDirections = static_cast<int>(Direction::Number_of_dirs);
}

template <typename Validation>
std::vector<Move> MoveRunnerFirst<Validation>::gatherMoves(
    const Point dimensions,
    const std::shared_ptr<BoardState> &currentState,
    const std::vector<Point> &invalidPositions,
    Direction previousDirection)
{
    std::vector<Move> newMoves{};

    auto moveBlockIfPossible = [&](const Block &block)
    {
        for (auto dir = 0; dir < numberOfDirections; ++dir)
        {
            if (dir != static_cast<int>(previousDirection))
            {
                const auto newBlock = move(block, static_cast<Direction>(dir));
                if (Validation::validBlockPosition(newBlock, dimensions, *currentState, invalidPositions))
                {
                    newMoves.emplace_back(currentState, block, static_cast<Direction>(dir));
                }
            }
        }
    };

    // Move runner first
    moveBlockIfPossible(currentState->m_runner);

    // Move other blocks second
    for (const auto &block : currentState->m_blocks)
    {
        moveBlockIfPossible(block);
    }
    return newMoves;
};

template
std::vector<Move> MoveRunnerFirst<DefaultMoveValidation>::gatherMoves(
    const Point dimensions,
    const std::shared_ptr<BoardState> &currentState,
    const std::vector<Point> &invalidPositions,
    Direction previousDirection);