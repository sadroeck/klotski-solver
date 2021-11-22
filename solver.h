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
	std::list<Puzzle> solve(std::ostream* benchmarkOut = nullptr);

private:
	std::list<Puzzle> solveBenchmark(std::ostream* benchmarkOut);
	std::list<Puzzle> solveFast();

	//format solution using solver data
	std::list<Puzzle> solution(std::shared_ptr<BoardState> firstBoardState, std::shared_ptr<BoardState> lastBoardState);

	const Puzzle puzzle;
	BoardHasher<HashType> hasher;

	// Stores all possible moves to explore
	std::list<Move> possibleMoveStack;

	// Stores the number of moves from the starting state
	std::unordered_map<HashType, NumberOfMovesType> knownPaths;

	std::unordered_map<HashType, std::vector<std::pair<HashType, std::shared_ptr<BoardState>>>> parentsOf;

	const MoveDiscovery& moveDiscovery;
};

