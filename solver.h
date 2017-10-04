#pragma once

#include <list>
#include <unordered_map>
#include <memory>

#include "BoardHasher.h"
#include "MoveDiscovery.h"
#include "MoveValidation.h"


template <int BlockCount>
bool isSolution(const BoardState<BlockCount>& state, const Block &goal)
{
    return state.m_runner.m_startX == goal.m_startX
        && state.m_runner.m_startY == goal.m_startY;
};

template <int BlockCount>
bool containsMove(const std::list<Move<BlockCount>> &moves, const Move<BlockCount> &move)
{
    return std::any_of(begin(moves), end(moves), [&](const auto &existingMove)
    {
        return existingMove.m_state.get() == move.m_state.get()
            && existingMove.m_directionToMove == move.m_directionToMove
            && same(existingMove.m_block, move.m_block);
    });
};

template <
    int BlockCount,
    typename MoveDiscovery
>
class Solver
{
public:
    using BoardStateId = std::string;
    using MovesFromStart = typename std::remove_const<decltype(BoardState<BlockCount>::m_numberOfMovesFromStart)>::type;

public:
    Solver(const Puzzle<BlockCount> &puzzle)
        : m_puzzle{ puzzle }
    {
        const auto initialMoves = MoveDiscovery::gatherMoves(
            puzzle.m_dimensions,
            std::make_shared<BoardState<BlockCount>>(m_puzzle.m_initialState),
            puzzle.m_forbiddenSpots);
        for (auto &move : initialMoves)
        {
            m_possibleMoves.push_back(std::move(move));
        }
    }

    ~Solver() = default;

    MovesFromStart solve()
    {
        while (!m_possibleMoves.empty())
        {
            auto firstMove = begin(m_possibleMoves);
            auto stateAfterMove = (*firstMove)();

            // Note: invalidates firstMove
            m_possibleMoves.pop_front();

            if (isSolution(stateAfterMove, m_puzzle.m_goal))
            {
                // TODO remove
                print(Puzzle<BlockCount>{ m_puzzle.m_dimensions, m_puzzle.m_goal, m_puzzle.m_forbiddenSpots, stateAfterMove });
                std::cout << "distance: " << stateAfterMove.m_numberOfMovesFromStart << std::endl;

                return stateAfterMove.m_numberOfMovesFromStart;
            }

            const auto hash = BoardHasher<BoardStateId>::hash(stateAfterMove);
            auto &distance = m_knownPaths[hash];

            if (distance == 0) // New path
                /*|| (distance > 0 // Shorter path
                    && distance > stateAfterMove.m_numberOfMovesFromStart))*/
            {
                distance = stateAfterMove.m_numberOfMovesFromStart;

                // TODO remove
                /*if (distance > 12)
                {
                    print(Puzzle{ m_puzzle.m_dimensions, m_puzzle.m_goal, m_puzzle.m_forbiddenSpots, stateAfterMove });
                    std::cout << "Hash: " << hash << " distance: " << distance << std::endl;
                }*/

                /*if (hash.size() > 20 && hash[19] != '3' && hash[20] != '0')
                {
                    print(Puzzle{ m_puzzle.m_dimensions, m_puzzle.m_goal, m_puzzle.m_forbiddenSpots, stateAfterMove });
                    std::cout << "Hash: " << hash << " distance: " << distance << std::endl;
                }*/

                /*if (hash == "@10A00B02C14D25E15F30G12H22I04")
                {
                    const auto newMoves = MoveDiscovery<>::gatherMoves(
                        m_puzzle.m_dimensions,
                        std::make_shared<BoardState>(std::move(stateAfterMove)),
                        m_puzzle.m_forbiddenSpots);
                    std::cout << "test";
                }*/

                // Queue follow-up moves
               const auto newMoves = MoveDiscovery::gatherMoves(
                    m_puzzle.m_dimensions,
                    std::make_shared<BoardState<BlockCount>>(std::move(stateAfterMove)),
                    m_puzzle.m_forbiddenSpots);

                for (auto &move : newMoves)
                {
                    //if (!containsMove(m_possibleMoves, move))
                    {
                        m_possibleMoves.push_back(std::move(move));
                    }
                }
            }
        }

        return -1;
    }

    // Retrieves all currently queued moves
    const std::list<Move<BlockCount>>& possibleMoves()
    {
        return m_possibleMoves;
    }

private:
    const Puzzle<BlockCount> m_puzzle;

    // Stores all possible moves to explore
    std::list<Move<BlockCount>> m_possibleMoves;

    // Stores the number of moves from the starting state
    std::unordered_map<BoardStateId, MovesFromStart> m_knownPaths;
};

template <int BlockCount, typename MoveDiscovery = MoveRunnerFirst<>>
Solver<BlockCount, MoveDiscovery> makeSolver(const Puzzle<BlockCount> &puzzle)
{
    return Solver<BlockCount, MoveDiscovery>{ puzzle };
}
