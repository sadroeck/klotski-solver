#pragma once

#include "BoardHasher.h"
#include "MoveDiscovery.h"

#include <list>
#include <unordered_map>

class Solver {
public:
	using HashType = int;
	using NumberOfMovesType = typename std::remove_const<decltype(BoardState::numberOfMoves)>::type;

public:
	Solver(const Puzzle& puzzle, const MoveDiscovery& moveDiscovery);
	~Solver() = default;
	std::list<Puzzle> solve(std::ostream* debugOut = nullptr);

private:
	std::list<Puzzle> solveDebug(std::ostream* debugOut);
	std::list<Puzzle> solveFast();
	std::list<Puzzle> solution(const BoardState& firstBoardState, const BoardState& lastBoardState);

	const Puzzle puzzle;
	BoardHasher<HashType> hasher;

	// Stores all possible moves to explore
	std::list<Move> possibleMoveStack;

	// Stores the number of moves from the starting state
	std::unordered_map<HashType, NumberOfMovesType> knownPaths;

	// Testing rermove me
	std::unordered_map<HashType, std::vector<std::pair<HashType, BoardState>>> parentsOf;

	const MoveDiscovery& moveDiscovery;
};

