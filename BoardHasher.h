#pragma once

#include <numeric>
#include <random>
#include <vector>

#include "puzzle.h"

// Need a hash that's both order invariant & block-id invariant
// i.e. 2 blocks with different ids at the same position should still hash the same
// i.e. hash should be the same regardless of the order in which the blocks on the board are hashed

// As we only update 1 block at a time, this would be perfect to update incrementally
// But i'll add this to the TODO list :-) ...

struct BlockSizeType
{
    int width;
    int height;
    bool operator==(const BlockSizeType &other)
    {
        return width == other.width && height == other.height;
    }
};

template <typename HashType = int>
struct BoardHasher
{
    // TODO: provide implementation for other HashTypes
    template <int BlockCount>
    HashType hash(const BoardState<BlockCount> &state)
    {
        HashType result{};
        result ^= runnerCode(state.m_runner);
        for (const auto &block : state.m_blocks)
        {
            result ^= blockStateCode(block);
        }
        return result;
    };

    template <int BlockCount>
    BoardHasher(const Puzzle<BlockCount> &puzzle)
        : m_width{ puzzle.m_dimensions.m_x }
        , m_height{ puzzle.m_dimensions.m_y }
        , m_codes{}
        , m_blockTypes{}
    {
        // Should use unordered set here, but compiler complains
        // std::unordered_set<BlockSizeType> uniqueBlocks;
        for (const auto &block : puzzle.m_initialState.m_blocks)
        {
            const BlockSizeType blockType{ block.m_sizeX, block.m_sizeY };
            if (end(m_blockTypes) == std::find(begin(m_blockTypes), end(m_blockTypes), blockType))
            {
                m_blockTypes.push_back(blockType);
            }
        }

        std::random_device device{};
        std::mt19937 generator{ device() };
        std::uniform_int_distribution<HashType> distribution{};

        const auto numberOfStates = (m_blockTypes.size() + 1) * puzzle.m_dimensions.m_x * puzzle.m_dimensions.m_y;
        for (auto i = 0u; i < numberOfStates; ++i)
        {
            m_codes.push_back(distribution(generator));
        }
    }

private:
    int blockStateCode(const Block &block)
    {
        const auto blockType = std::distance(begin(m_blockTypes), std::find_if(
            begin(m_blockTypes),
            end(m_blockTypes),
            [&](const auto &type)
        {
            return type.width == block.m_sizeX && type.height == block.m_sizeY;
        }));

        // FIrst blockType is runner, so index offset of the other blocks is 1
        return m_codes[
            ((blockType + 1) * (m_width * m_height))
                + (block.m_startX * m_height)
                + block.m_startY];
    }

    int runnerCode(const Block &runner)
    {
        // First blockType is the runner
        return m_codes[(runner.m_startX * m_height) + runner.m_startY];
    }

private:
    // Dimensions of the playing field
    int m_width;
    int m_height;

    // unique code for each blocktype positioned on the board
    // Note: indexing scheme = [blockType][x][y]
    std::vector<HashType> m_codes;

    // Unique code for each block size
    std::vector<BlockSizeType> m_blockTypes;
    
};