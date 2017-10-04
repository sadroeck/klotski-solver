#pragma once

#include <list>
#include <unordered_map>
#include <memory>
#include <chrono>

#include "BoardHasher.h"
#include "MoveDiscovery.h"
#include "MoveValidation.h"

#define TIMING_ON 1


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
#ifdef TIMING_ON
        auto globalTime = std::chrono::high_resolution_clock::now();
        auto pickMoveTime = std::chrono::high_resolution_clock::duration{};
        auto solutionCheckTime = std::chrono::high_resolution_clock::duration{};
        auto hashTime = std::chrono::high_resolution_clock::duration{};
        auto lookupTime = std::chrono::high_resolution_clock::duration{};
        auto gatherMovesTime = std::chrono::high_resolution_clock::duration{};
        auto insertMovesTime = std::chrono::high_resolution_clock::duration{};
#endif // TIMING_ON


        while (!m_possibleMoves.empty())
        {
#ifdef TIMING_ON
            auto start_time = std::chrono::high_resolution_clock::now();
#endif // TIMING_ON

            auto firstMove = begin(m_possibleMoves);
            auto stateAfterMove = (*firstMove)();

            // Note: invalidates firstMove
            m_possibleMoves.pop_front();

#ifdef TIMING_ON
            auto end_time = std::chrono::high_resolution_clock::now();
            pickMoveTime += (end_time - start_time);

            start_time = std::chrono::high_resolution_clock::now();
#endif // TIMING_ON

            if (isSolution(stateAfterMove, m_puzzle.m_goal))
            {
                print(Puzzle<BlockCount>{ m_puzzle.m_dimensions, m_puzzle.m_goal, m_puzzle.m_forbiddenSpots, stateAfterMove });
                std::cout << "distance: " << stateAfterMove.m_numberOfMovesFromStart << std::endl;

#ifdef TIMING_ON
                const auto now = std::chrono::high_resolution_clock::now();
                std::cout << "Total time was: " << std::chrono::duration_cast<std::chrono::milliseconds>(now - globalTime).count() << std::endl;
                std::cout << "pickMoveTime " << std::chrono::duration_cast<std::chrono::milliseconds>(pickMoveTime).count() << std::endl;
                std::cout << "solutionCheckTime " << std::chrono::duration_cast<std::chrono::milliseconds>(solutionCheckTime).count() << std::endl;
                std::cout << "hashTime " << std::chrono::duration_cast<std::chrono::milliseconds>(hashTime).count() << std::endl;
                std::cout << "lookupTime " << std::chrono::duration_cast<std::chrono::milliseconds>(lookupTime).count() << std::endl;
                std::cout << "gatherMovesTime " << std::chrono::duration_cast<std::chrono::milliseconds>(gatherMovesTime).count() << std::endl;
                std::cout << "insertMovesTime " << std::chrono::duration_cast<std::chrono::milliseconds>(insertMovesTime).count() << std::endl;
#endif // TIMING_ON

                return stateAfterMove.m_numberOfMovesFromStart;
            }

#ifdef TIMING_ON
            end_time = std::chrono::high_resolution_clock::now();

            solutionCheckTime += (end_time - start_time);
            start_time = std::chrono::high_resolution_clock::now();
#endif // TIMING_ON

            const auto hash = BoardHasher<BoardStateId>::hash(stateAfterMove);

#ifdef TIMING_ON
            end_time = std::chrono::high_resolution_clock::now();
            hashTime += (end_time - start_time);

            start_time = std::chrono::high_resolution_clock::now();
#endif // TIMING_ON

            auto &distance = m_knownPaths[hash];
            
#ifdef TIMING_ON
            end_time = std::chrono::high_resolution_clock::now();
            lookupTime += (end_time - start_time);
#endif // TIMING_ON

            if (distance == 0) // New path
                /*|| (distance > 0 // Shorter path
                    && distance > stateAfterMove.m_numberOfMovesFromStart))*/
            {
                distance = stateAfterMove.m_numberOfMovesFromStart;

#ifdef TIMING_ON
                start_time = std::chrono::high_resolution_clock::now();
#endif // TIMING_ON

                // Queue follow-up moves
               const auto newMoves = MoveDiscovery::gatherMoves(
                    m_puzzle.m_dimensions,
                    std::make_shared<BoardState<BlockCount>>(std::move(stateAfterMove)),
                    m_puzzle.m_forbiddenSpots);

#ifdef TIMING_ON
               end_time = std::chrono::high_resolution_clock::now();
               gatherMovesTime += (end_time - start_time);

               start_time = std::chrono::high_resolution_clock::now();
#endif // TIMING_ON

                for (auto &move : newMoves)
                {
                    if (!containsMove(m_possibleMoves, move))
                    {
                        m_possibleMoves.push_back(std::move(move));
                    }
                }

#ifdef TIMING_ON
                end_time = std::chrono::high_resolution_clock::now();
                insertMovesTime += (end_time - start_time);
#endif // TIMING_ON

#ifdef TIMING_ON
                if (distance > 25)
                {
                    const auto now = std::chrono::high_resolution_clock::now();
                    std::cout << "Total time was: " << std::chrono::duration_cast<std::chrono::milliseconds>(now - globalTime).count() << std::endl;
                    std::cout << "solutionCheckTime " << std::chrono::duration_cast<std::chrono::milliseconds>(solutionCheckTime).count() << std::endl;
                    std::cout << "hashTime " << std::chrono::duration_cast<std::chrono::milliseconds>(hashTime).count() << std::endl;
                    std::cout << "lookupTime " << std::chrono::duration_cast<std::chrono::milliseconds>(lookupTime).count() << std::endl;
                    std::cout << "gatherMovesTime " << std::chrono::duration_cast<std::chrono::milliseconds>(gatherMovesTime).count() << std::endl;
                    std::cout << "insertMovesTime " << std::chrono::duration_cast<std::chrono::milliseconds>(insertMovesTime).count() << std::endl;
#endif // TIMING_ON
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
