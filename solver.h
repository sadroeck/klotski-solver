#pragma once

#include "BoardHasher.h"
#include "MoveDiscovery.h"

#include <list>
#include <unordered_map>

class Solver {
public:
	using HashType = int;
	using MoveNumberType = typename std::remove_const<decltype(BoardState::moveNumber)>::type;

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
	std::list<Move> movesToExploreSoon;
	std::list<Move> movesToExploreLater;

	std::unordered_map<HashType, std::pair<HashType, std::shared_ptr<BoardState>>> parentOf;

	const MoveDiscovery& moveDiscovery;
};

