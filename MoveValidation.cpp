#include "MoveValidation.h"

#include <algorithm>

namespace
{
    bool overlapsWithInvalidSpaces(const Block &block, const std::vector<Point> &invalidSpots)
    {
        return std::any_of(
            begin(invalidSpots),
            end(invalidSpots),
            [&](const auto &spot)
        {
            return overlaps(block, Block{ spot.m_x, spot.m_y, 1, 1, block.id});
        });
    }

    bool overlapsWithBorder(const Block &block, const Point &dims)
    {
        return block.m_startX < 0 ||
            block.m_startY < 0 ||
            (block.m_startX + block.m_sizeX) > dims.m_x ||
            (block.m_startY + block.m_sizeY) > dims.m_y;
    }

    bool overlapsWithOtherBlocks(const Block &block, const BoardState &board)
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
    }
}

bool DefaultMoveValidation::validBlockPosition(
    const Block &block,
    const Point &dims,
    const BoardState &boardState,
    const std::vector<Point> invalidPositions)
{
    return !overlapsWithBorder(block, dims)
        && !overlapsWithInvalidSpaces(block, invalidPositions)
        && !overlapsWithOtherBlocks(block, boardState);
}
