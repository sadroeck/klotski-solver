#pragma once

#include <list>
#include <unordered_map>
#include <memory>

#include "BoardHasher.h"
#include "MoveDiscovery.h"
#include "MoveValidation.h"


bool isSolution(const BoardState& state, const Block &goal);
bool containsMove(const std::list<Move> &moves, const Move &move);

template <
    template <typename MoveValidation = DefaultMoveValidation> typename MoveDiscovery
>
class Solver
{
public:
    using BoardStateId = std::string;
    using MovesFromStart = typename std::remove_const<decltype(BoardState::m_numberOfMovesFromStart)>::type;

public:
    Solver(const Puzzle &puzzle)
        : m_puzzle{ puzzle }
    {
        const auto initialMoves = MoveDiscovery<>::gatherMoves(
            puzzle.m_dimensions,
            std::make_shared<BoardState>(m_puzzle.m_initialState),
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
                print(Puzzle{ m_puzzle.m_dimensions, m_puzzle.m_goal, m_puzzle.m_forbiddenSpots, stateAfterMove });
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
                const auto newMoves = MoveDiscovery<>::gatherMoves(
                    m_puzzle.m_dimensions,
                    std::make_shared<BoardState>(std::move(stateAfterMove)),
                    m_puzzle.m_forbiddenSpots);

                for (auto &move : newMoves)
                {
                    if (!containsMove(m_possibleMoves, move))
                    {
                        m_possibleMoves.push_back(std::move(move));
                    }
                }
            }
        }

        return -1;
    };

    // Retrieves all currently queued moves
    const std::list<Move>& possibleMoves()
    {
        return m_possibleMoves;
    }

private:
    const Puzzle m_puzzle;

    // Stores all possible moves to explore
    std::list<Move> m_possibleMoves;

    // Stores the number of moves from the starting state
    std::unordered_map<BoardStateId, MovesFromStart> m_knownPaths;
};