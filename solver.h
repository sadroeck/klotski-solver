#pragma once

#include <chrono>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

#include "BoardHasher.h"
#include "MoveDiscovery.h"
#include "MoveValidation.h"
#include "printer.h"

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
    using BoardStateId = int;
    using MovesFromStart = typename std::remove_const<decltype(BoardState<BlockCount>::m_numberOfMovesFromStart)>::type;

public:
    Solver(const Puzzle<BlockCount> &puzzle)
        : m_puzzle{ puzzle }
        , m_hasher{ puzzle }
    {
        std::cout << "Solving:" << std::endl;
        print(Puzzle<BlockCount>{ m_puzzle.m_dimensions, m_puzzle.m_goal, m_puzzle.m_forbiddenSpots, m_puzzle.m_initialState });

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

    template<bool ShowMoves = false>
    MovesFromStart solve();

    template<>
    MovesFromStart solve<true>()
    {
        auto globalTime = std::chrono::high_resolution_clock::now();
        auto pickMoveTime = std::chrono::high_resolution_clock::duration{};
        auto solutionCheckTime = std::chrono::high_resolution_clock::duration{};
        auto hashTime = std::chrono::high_resolution_clock::duration{};
        auto lookupTime = std::chrono::high_resolution_clock::duration{};
        auto gatherMovesTime = std::chrono::high_resolution_clock::duration{};
        auto insertMovesTime = std::chrono::high_resolution_clock::duration{};
        
        while (!m_possibleMoves.empty())
        {
            auto start_time = std::chrono::high_resolution_clock::now();

            auto firstMove = begin(m_possibleMoves);
            auto stateAfterMove = (*firstMove)();

            auto tempCopy = *firstMove;

            // Note: invalidates firstMove
            m_possibleMoves.pop_front();

            auto end_time = std::chrono::high_resolution_clock::now();
            pickMoveTime += (end_time - start_time);
            start_time = std::chrono::high_resolution_clock::now();

            const auto previousHash = m_hasher.hash(*tempCopy.m_state);

            if (isSolution(stateAfterMove, m_puzzle.m_goal))
            {
                std::cout << "Solution is:" << std::endl;
                print(Puzzle<BlockCount>{ m_puzzle.m_dimensions, m_puzzle.m_goal, m_puzzle.m_forbiddenSpots, stateAfterMove });
                std::cout << "distance: " << stateAfterMove.m_numberOfMovesFromStart << std::endl;
                const auto hash = m_hasher.hash(stateAfterMove);
                std::cout << "Final hash: " << hash << std::endl;

                const auto initialHash = m_hasher.hash(m_puzzle.m_initialState);
                int stopHash = previousHash;
                while (stopHash != 0 && stopHash != initialHash)
                {
                    auto start = parentsOf[stopHash];
                    if (start.size() > 0 && start[0].first != 0)
                    {
                        std::cout << start[0].first << std::endl;
                        print(Puzzle<BlockCount>{ m_puzzle.m_dimensions, m_puzzle.m_goal, m_puzzle.m_forbiddenSpots, start[0].second });
                        std::cout << std::endl << " -------------- " << std::endl;

                        stopHash = start[0].first;
                    }
                }

                std::cout << initialHash << std::endl;
                print(Puzzle<BlockCount>{ m_puzzle.m_dimensions, m_puzzle.m_goal, m_puzzle.m_forbiddenSpots, m_puzzle.m_initialState });
                std::cout << std::endl << " -------------- " << std::endl;

                const auto now = std::chrono::high_resolution_clock::now();
                std::cout << "Total time was: " << std::chrono::duration_cast<std::chrono::milliseconds>(now - globalTime).count() << std::endl;
                std::cout << "pickMoveTime " << std::chrono::duration_cast<std::chrono::milliseconds>(pickMoveTime).count() << std::endl;
                std::cout << "solutionCheckTime " << std::chrono::duration_cast<std::chrono::milliseconds>(solutionCheckTime).count() << std::endl;
                std::cout << "hashTime " << std::chrono::duration_cast<std::chrono::milliseconds>(hashTime).count() << std::endl;
                std::cout << "lookupTime " << std::chrono::duration_cast<std::chrono::milliseconds>(lookupTime).count() << std::endl;
                std::cout << "gatherMovesTime " << std::chrono::duration_cast<std::chrono::milliseconds>(gatherMovesTime).count() << std::endl;
                std::cout << "insertMovesTime " << std::chrono::duration_cast<std::chrono::milliseconds>(insertMovesTime).count() << std::endl;

                return stateAfterMove.m_numberOfMovesFromStart;
            }

            end_time = std::chrono::high_resolution_clock::now();
            solutionCheckTime += (end_time - start_time);
            start_time = std::chrono::high_resolution_clock::now();

            const auto hash = m_hasher.hash(stateAfterMove);

            end_time = std::chrono::high_resolution_clock::now();
            hashTime += (end_time - start_time);
            start_time = std::chrono::high_resolution_clock::now();

            auto &distance = m_knownPaths[hash];
            
            end_time = std::chrono::high_resolution_clock::now();
            lookupTime += (end_time - start_time);

            if (distance == 0) // New path
                /*|| (distance > 0 // Shorter path
                    && distance > stateAfterMove.m_numberOfMovesFromStart))*/
            {
                auto &parents = parentsOf[hash];
                parents.push_back({ previousHash, stateAfterMove });

                distance = stateAfterMove.m_numberOfMovesFromStart;

                start_time = std::chrono::high_resolution_clock::now();

                // Queue follow-up moves
               const auto newMoves = MoveDiscovery::gatherMoves(
                    m_puzzle.m_dimensions,
                    std::make_shared<BoardState<BlockCount>>(std::move(stateAfterMove)),
                    m_puzzle.m_forbiddenSpots);

               end_time = std::chrono::high_resolution_clock::now();
               gatherMovesTime += (end_time - start_time);
               start_time = std::chrono::high_resolution_clock::now();

                for (auto &move : newMoves)
                {
                    m_possibleMoves.push_back(std::move(move));
                }

                end_time = std::chrono::high_resolution_clock::now();
                insertMovesTime += (end_time - start_time);
            }
        }

        return -1;
    }

    template<>
    MovesFromStart solve<false>()
    {
        while (!m_possibleMoves.empty())
        {
            auto firstMove = begin(m_possibleMoves);
            auto stateAfterMove = (*firstMove)();
            // Note: invalidates firstMove
            m_possibleMoves.pop_front();

            if (isSolution(stateAfterMove, m_puzzle.m_goal))
            {
                return stateAfterMove.m_numberOfMovesFromStart;
            }

            const auto hash = m_hasher.hash(stateAfterMove);

            auto &distance = m_knownPaths[hash];

            if (distance == 0) // New path
                /*|| (distance > 0 // Shorter path
                && distance > stateAfterMove.m_numberOfMovesFromStart))*/
            {
                distance = stateAfterMove.m_numberOfMovesFromStart;

                // Queue follow-up moves
                const auto newMoves = MoveDiscovery::gatherMoves(
                    m_puzzle.m_dimensions,
                    std::make_shared<BoardState<BlockCount>>(std::move(stateAfterMove)),
                    m_puzzle.m_forbiddenSpots);

                for (auto &move : newMoves)
                {
                    m_possibleMoves.push_back(std::move(move));
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
    BoardHasher<> m_hasher;

    // Stores all possible moves to explore
    std::list<Move<BlockCount>> m_possibleMoves;

    // Stores the number of moves from the starting state
    std::unordered_map<BoardStateId, MovesFromStart> m_knownPaths;

    // Testing rermove me
    using State = std::pair<BoardStateId, BoardState<BlockCount>>;
    std::unordered_map<BoardStateId, std::vector<State>> parentsOf;
};

template <int BlockCount, typename MoveDiscovery = MoveRunnerFirst<>>
Solver<BlockCount, MoveDiscovery> makeSolver(const Puzzle<BlockCount> &puzzle)
{
    return Solver<BlockCount, MoveDiscovery>{ puzzle };
}
