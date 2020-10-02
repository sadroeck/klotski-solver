#include "puzzle.h"

BoardState Move::proceed() {
	if (block.id == boardState->runner.id) {
		// Block to move is the runner
		return BoardState {
			boardState->numberOfMoves + 1,
			boardState->runner.move(directionToMove),
			boardState->blocks,
		};

	} else {
		// Block to move is one of the other blocks
		// Notes: assumes linear distance in storage container
		const auto blockIndex = &block - &boardState->blocks[0];

		auto newBlocks = boardState->blocks;
		newBlocks[blockIndex] = block.move(directionToMove);

		return BoardState {
			boardState->numberOfMoves + 1,
			boardState->runner,
			std::move(newBlocks),
		};

	}
}

Move::Move(std::shared_ptr<BoardState> boardState, const Block& block, Direction dir)
	: boardState{ boardState }
	, block{ block }
	, directionToMove{ dir }
{
}

PuzzleValidation Puzzle::validate() const {
	const auto& blockSize = boardState.blocks.size();

	for (size_t i = 0; i < blockSize; ++i) {
		for (size_t j = 0; j < blockSize; ++j) {
			if (i < j) {
				if (boardState.blocks[i].overlaps(boardState.blocks[j])) {
					return blocksOverlaps;
				}
				if (boardState.blocks[i].id == boardState.blocks[j].id) {
					return blocksWithSameId;
				}
			}
		}

		if (boardState.blocks[i].overlaps(this->boardState.runner)) {
			return runnerOverlaps;
		}
		if (boardState.runner.id == boardState.blocks[i].id) {
			return runnerWithSameId;
		}
	}

	return PuzzleValidation::valid;
}

std::string Puzzle::validationString(const PuzzleValidation& validation) {
	switch (validation) {
		case valid:            return "puzzle is valid";
		case blocksOverlaps:   return "tow blocks overlaps";
		case runnerOverlaps:   return "runner and block overlaps";
		case blocksWithSameId: return "tow blocks has the same id";
		case runnerWithSameId: return "runner and block has the same id";
		default:
			throw std::runtime_error("Unknown validation " + static_cast<int>(validation));
	}
}

