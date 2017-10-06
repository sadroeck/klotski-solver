#pragma once

#include "solver.h"
#include <algorithm>
#include <iostream>

template <int BlockCount>
void print(const Puzzle<BlockCount> &puzzle)
{
    std::vector<std::vector<char>> layout(
        puzzle.m_dimensions.m_y + 2,
        std::vector<char>(puzzle.m_dimensions.m_x + 2));

    auto fillBlock = [&](const auto &block)
    {
        for (auto x = block.m_startX; x < block.m_startX + block.m_sizeX; ++x)
        {
            for (auto y = block.m_startY; y < block.m_startY + block.m_sizeY; ++y)
            {
                layout[y + 1][x + 1] = block.id[0];
            }
        }
    };

    // Print border blocks
    for (auto y = 0; y < puzzle.m_dimensions.m_y + 2; ++y)
    {
        layout[y][0] = '#';
        layout[y][puzzle.m_dimensions.m_x + 1] = '#';
    }
    for (auto x = 1; x < puzzle.m_dimensions.m_x + 1; ++x)
    {
        layout[0][x] = '#';
        layout[puzzle.m_dimensions.m_y + 1][x] = '#';
    }
    for (const auto &forbiddenPoint : puzzle.m_forbiddenSpots)
    {
        layout[forbiddenPoint.m_y + 1][forbiddenPoint.m_x + 1] = '#';
    }

    fillBlock(puzzle.m_goal);
    fillBlock(puzzle.m_initialState.m_runner);

    // Print other blocks
    for (const auto &contentBlock : puzzle.m_initialState.m_blocks)
    {
        fillBlock(contentBlock);
    }

    for (auto y = 0u; y < layout.size(); ++y)
    {
        for (auto x = 0u; x < layout[0].size(); ++x)
        {
            std::cout << layout[y][x];
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;
};
