#pragma once

#include <vector>
#include <memory>

#include "block.h"
#include "puzzle.h"
#include "MoveValidation.h"

template <typename Validation = DefaultMoveValidation>
struct MoveRunnerFirst
{
    template <int BlockCount>
    static std::vector<Move<BlockCount>> gatherMoves(
        const Point dimensions,
        const std::shared_ptr<BoardState<BlockCount>> &currentState,
        const std::vector<Point> &invalidPositions,
        Direction previousDirection = Direction::Number_of_dirs)
    {
        std::vector<Move<BlockCount>> newMoves{};

        auto moveBlockIfPossible = [&](const Block &block)
        {
            for (auto dir = 0; dir < static_cast<int>(Direction::Number_of_dirs); ++dir)
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
    }
};