#include "solver.h"

#include <algorithm>

bool isSolution(const BoardState& state, const Block &goal)
{
    return state.m_runner.m_startX == goal.m_startX
        && state.m_runner.m_startY == goal.m_startY;
}

bool containsMove(const std::list<Move> &moves, const Move &move)
{
    return std::any_of(begin(moves), end(moves), [&](const Move &existingMove)
    {
        return existingMove.m_state.get() == move.m_state.get()
            && existingMove.m_directionToMove == move.m_directionToMove
            && same(existingMove.m_block, move.m_block);
    });
}