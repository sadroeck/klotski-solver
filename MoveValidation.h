#pragma once

#include "puzzle.h"

// Checks if the proposed position of the block against the current state of the board is a valid one.
bool validBlockPosition(
	const Block &block,
	const Point &dims,
	const BoardState& boardState,
	const std::set<Point> invalidPositions
);

