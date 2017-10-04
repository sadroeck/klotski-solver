#include "puzzle.h"

#include <cassert>

BoardState Move::operator()()
{
    if (same(m_block, m_state->m_runner))
    {
        // Block to move is the runner
        return BoardState
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

        return BoardState
        {
            m_state->m_numberOfMovesFromStart + 1,
            m_state->m_runner,
            std::move(newBlocks),
        };
    }
}

bool valid(const Puzzle &puzzle)
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