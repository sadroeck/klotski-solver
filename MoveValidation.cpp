#include "MoveValidation.h"

#include <algorithm>

namespace detail
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
}


