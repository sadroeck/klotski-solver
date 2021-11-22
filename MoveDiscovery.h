#pragma once

#include "puzzle.h"

#include <vector>
#include <set>

class MoveDiscovery {
public:
	virtual std::vector<Move> gatherMoves(
		const Point dimensions,
		const std::shared_ptr<BoardState>& boardState,
		const std::set<Point>& invalidPositions
	) const = 0;
};

class MoveRunnerFirst : public MoveDiscovery {
public:
	virtual std::vector<Move> gatherMoves(
		const Point dimensions,
		const std::shared_ptr<BoardState>& boardState,
		const std::set<Point>& invalidPositions
	) const;
};

