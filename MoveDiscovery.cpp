#include "MoveDiscovery.h"

#include "MoveValidation.h"

std::vector<Move> MoveRunnerFirst::gatherMoves(
	const Point dimensions,
	const std::shared_ptr<BoardState>& boardState,
	const std::set<Point>& invalidPositions,
	const Block* lastMovedBlock
) const {
	std::vector<Move> result{};

	auto moveBlockIfPossible = [&](const Block& block) {
		const size_t moveEffort = ((nullptr != lastMovedBlock) && (lastMovedBlock->id == block.id)) ? 0 : 1;
		for (auto dir = 0; dir < static_cast<int>(Direction::Number_of_dirs); ++dir) {
				if (validBlockPosition(
					block.move(static_cast<Direction>(dir)),
					dimensions,
					*boardState,
					invalidPositions
				)) {
					result.emplace_back(boardState, block, moveEffort, static_cast<Direction>(dir));
				}
			}
	};

	// Move runner first
	moveBlockIfPossible(boardState->runner);

	// Move other blocks second
	for (const auto& block : boardState->blocks) {
		moveBlockIfPossible(block);
	}
	return result;
}

