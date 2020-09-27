#include "MoveValidation.h"

#include <stdexcept>
#include <algorithm>

namespace detail {

	bool overlapsWithInvalidSpaces(const Block& block, const std::set<Point>& invalidSpotSet) {
		return std::any_of(
			begin(invalidSpotSet),
			end(invalidSpotSet),
			[&](const Point& invalidSpot) {
				return block.overlaps(invalidSpot);
			}
		);
	}

	bool overlapsWithBorder(const Block& block, const Point& dims) {
		if (
			(block.shift.x < 0)
			||
			(block.shift.y < 0)
		) {
				return true;
		}
		for (const auto& blockPoint : block.pointSet) {
			if (
				((block.shift.x + blockPoint.x) >= dims.x)
				||
				((block.shift.y + blockPoint.y) >= dims.y)
			) {
				return true;
			}
		}
		return false;
	}

	bool overlapsWithOtherBlocks(const Block& block, const BoardState& board) {
		return
			((block.id != board.runner.id) && block.overlaps(board.runner))
			||
			std::any_of(
				begin(board.blocks),
				end(board.blocks),
				[&](const auto &other) { return (other.id != block.id) && block.overlaps(other); }
			)
		;
	}
}

bool validBlockPosition(
	const Block &block,
	const Point &dims,
	const BoardState& boardState,
	const std::set<Point> invalidPositions
) {
	return !detail::overlapsWithBorder(block, dims)
		&& !detail::overlapsWithInvalidSpaces(block, invalidPositions)
		&& !detail::overlapsWithOtherBlocks(block, boardState)
	;
}

