#pragma once

#include <numeric>
#include <string>

#include "puzzle.h"

// A very inefficient but simple hashing algorithm that's block-order invariant
// As we only make single-step updates to the board
// We could refine this to an incremental hashing algorithm
// But i'll add this to the TODO list :-) ...

template <typename HashType = std::string>
struct BoardHasher
{
    // TODO: provide implementation for other HashTypes
    template <int BlockCount>
    static HashType hash(const BoardState<BlockCount> &state)
    {
        auto runnerState = state.m_runner.id;
        runnerState += std::to_string(state.m_runner.m_startX);
        runnerState += std::to_string(state.m_runner.m_startY);

        return std::accumulate(
            begin(state.m_blocks),
            end(state.m_blocks),
            runnerState,
            [](std::string partial, const Block &block)
        {
            partial += block.id;
            partial += std::to_string(block.m_startX);
            partial += std::to_string(block.m_startY);
            return partial;
        });
    };
};