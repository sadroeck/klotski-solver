#pragma once

#include "puzzle.h"

namespace detail
{
    bool overlapsWithInvalidSpaces(const Block &block, const std::vector<Point> &invalidSpots);
    
    bool overlapsWithBorder(const Block &block, const Point &dims);
    
    template <int BlockCount>
    bool overlapsWithOtherBlocks(const Block &block, const BoardState<BlockCount> &board)
    {
        const auto overlapsRunner = same(block, board.m_runner) ? false : overlaps(block, board.m_runner);

        const auto overlapsOthers = std::any_of(
            begin(board.m_blocks),
            end(board.m_blocks),
            [&](const auto &other)
        {
            return same(other, block) ? false : overlaps(block, other);
        });

        return overlapsRunner || overlapsOthers;
    };
}

struct DefaultMoveValidation
{
    // Checks if the proposed position of the block against the current state of the board
    // is a valid one.
    // The position of the current block is ignored
    template <int BlockCount>
    static bool validBlockPosition(
        const Block &block,
        const Point &dims,
        const BoardState<BlockCount> &boardState,
        const std::vector<Point> invalidPositions)
    {
        return !detail::overlapsWithBorder(block, dims)
            && !detail::overlapsWithInvalidSpaces(block, invalidPositions)
            && !detail::overlapsWithOtherBlocks(block, boardState);
    }
};