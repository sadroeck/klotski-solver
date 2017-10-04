#include "block.h"

#include <algorithm>

// Does not take block dimensions into account
// Intended as a rough filtering step to see which blocks to try to move first
bool nextToFreeSpace(const Block &block, const std::vector<Point> &freeSpaces)
{
    return std::any_of(begin(freeSpaces), end(freeSpaces), [&](const auto &freeSpace)
    {
        const auto nextToInX = overlaps(block, Block{ freeSpace.m_x - 1, freeSpace.m_y, 1, 1, block.id})
            || overlaps(block, Block{ freeSpace.m_x + 1, freeSpace.m_y, 1, 1, block.id});
        const auto nextToInY = overlaps(block, Block{ freeSpace.m_x, freeSpace.m_y - 1, 1, 1, block.id})
            || overlaps(block, Block{ freeSpace.m_x, freeSpace.m_y + 1, 1, 1, block.id});

        return nextToInX ^ nextToInY;
    });
}

bool overlaps(const Block &left, const Block &right)
{
    const auto valueInRange = [](auto value, auto min, auto max)
    {
        return (value >= min) && (value <= max);
    };
    const auto xOverlap =
        valueInRange(left.m_startX, right.m_startX, right.m_startX + right.m_sizeX - 1)
        || valueInRange(right.m_startX, left.m_startX, left.m_startX + left.m_sizeX - 1);

    const auto yOverlap =
        valueInRange(left.m_startY, right.m_startY, right.m_startY + right.m_sizeY - 1)
        || valueInRange(right.m_startY, left.m_startY, left.m_startY + left.m_sizeY - 1);

    return xOverlap && yOverlap;
}

Block move(const Block &block, Direction dir)
{
    switch (dir)
    {
    case Up:
        return Block{ block.m_startX, block.m_startY - 1, block.m_sizeX, block.m_sizeY, block.id };
    case Down:
        return Block{ block.m_startX, block.m_startY + 1, block.m_sizeX, block.m_sizeY, block.id };
    case Left:
        return Block{ block.m_startX - 1, block.m_startY, block.m_sizeX, block.m_sizeY, block.id };
    case Right:
        return Block{ block.m_startX + 1, block.m_startY, block.m_sizeX, block.m_sizeY, block.id };
    }

    throw std::runtime_error("Unknown direction " + static_cast<int>(dir));
    return block;
}

bool equalPosition(const Block &left, const Block &right)
{
    return left.m_startX == right.m_startX
        && left.m_startY == right.m_startY;
}

bool same(const Block &left, const Block &right)
{
    return left.id == right.id;
}