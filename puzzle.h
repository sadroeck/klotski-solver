#pragma once

#include <array>
#include <cassert>
#include <memory>


#include "block.h"

template <int BlockCount>
struct BoardState
{
    constexpr static int blockCount = BlockCount;

    // Numer of moves made since the start position
    const int m_numberOfMovesFromStart;

    // Puzzle piece to be moved to the goal
    const Block m_runner;

    // all movable bystander blocks on the board
    const std::array<Block, BlockCount> m_blocks;
};

template <int BlockCount>
struct Move
{
    // State from which to move
    std::shared_ptr<BoardState<BlockCount>> m_state;

    // Block to be moved
    const Block &m_block;

    // Direction in which to move block
    Direction m_directionToMove;

    // Returns the BoardState after the move
    BoardState<BlockCount> operator()();

    Move(std::shared_ptr<BoardState<BlockCount>> state, const Block &block, Direction dir)
        : m_state{ state }
        , m_block{ block }
        , m_directionToMove{ dir }
    {
    }
};

template <int BlockCount>
struct Puzzle
{
    // Dimensions of the puzzle:
    // X: [ 0, width - 1]
    // Y: [ 0, height - 1]
    const Point m_dimensions;

    // Position of the runner required in order to finish the puzzle
    const Block m_goal;

    // Positions (in addition to regular border) where blocks cannot be placed
    const std::vector<Point> m_forbiddenSpots;

    // Initial position of all blocks on the board
    const BoardState<BlockCount> m_initialState;
};

template <int BlockCount>
bool valid(const Puzzle<BlockCount> &puzzle)
{
    const auto blockCount = puzzle.m_initialState.m_blocks.size();
    const auto &blocks = puzzle.m_initialState.m_blocks;
    bool valid = true;

    for (auto i = 0u; i < blockCount; ++i)
    {
        for (auto j = 0u; j < blockCount; ++j)
        {
            if (i != j)
            {
                valid = valid && !overlaps(blocks[i], blocks[j]);
                valid = valid && (blocks[i].id != blocks[j].id);
            }
        }

        valid = valid && !overlaps(blocks[i], puzzle.m_initialState.m_runner);
        valid = valid && (puzzle.m_goal.id != blocks[i].id);
        valid = valid && (puzzle.m_initialState.m_runner.id != blocks[i].id);
    }

    return valid;
}

template <int BlockCount>
BoardState<BlockCount> Move<BlockCount>::operator()()
{
    if (same(m_block, m_state->m_runner))
    {
        // Block to move is the runner
        return BoardState<BlockCount>
        {
            m_state->m_numberOfMovesFromStart + 1,
            move(m_state->m_runner, m_directionToMove),
            m_state->m_blocks,
        };
    }
    else
    {
        // Block to move is one of the other blocks
        // Notes: assumes linear distance in storage container
        const auto blockIndex = &m_block - &m_state->m_blocks[0];

        auto newBlocks = m_state->m_blocks;
        newBlocks[blockIndex] = move(m_block, m_directionToMove);

        return BoardState<BlockCount>
        {
            m_state->m_numberOfMovesFromStart + 1,
            m_state->m_runner,
            std::move(newBlocks),
        };
    }
}
