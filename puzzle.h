#pragma once

#include "block.h"

#include <vector>
#include <cassert>
#include <memory>
#include <sstream>

struct BoardState {
	// Numer of moves made since the start position
	const int numberOfMoves;

	// Puzzle piece to be moved to the goal
	const Block runner;

	// all movable bystander blocks on the board
	const std::vector<Block> blocks;
};

class Move {
public:
	// State from which to move
	std::shared_ptr<BoardState> boardState;

	// Block to be moved
	const Block &block;

	// Direction in which to move block
	Direction directionToMove;

	// Returns the BoardState after the move
	BoardState proceed();

	Move(std::shared_ptr<BoardState> boardState, const Block &block, Direction dir);
};

enum PuzzleValidation {
	valid,
	blocksOverlaps,
	runnerOverlaps,
	blocksWithSameId,
	runnerWithSameId,
	Number_of_validation
};

class Puzzle {
public:
	// Dimensions of the puzzle:
	const Point dimensions;

	// Position of the runner required in order to finish the puzzle
	const Point goal;

	// Positions (in addition to regular border) where blocks cannot be placed
	const std::set<Point> forbiddenSpots;

	// Initial position of all blocks on the board
	const BoardState boardState;

	PuzzleValidation validate() const;
	static std::string validationString(const PuzzleValidation& validation);
};

inline bool operator==(const Puzzle p1, const Puzzle p2) {
	return
		(p1.dimensions == p2.dimensions)
		&&
		(p1.forbiddenSpots == p2.forbiddenSpots)
		&&
		(p1.goal == p2.goal)
		&&
		(p1.boardState.runner == p2.boardState.runner)
		&&
		(p1.boardState.blocks == p2.boardState.blocks)
	;
}
inline bool operator!=(const Puzzle p1, const Puzzle p2) { return !(p1 == p2); }

