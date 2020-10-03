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
		return std::any_of(
			begin(block.pointSet),
			end(block.pointSet),
			[&](const Point& blockPoint) {
				return
					((block.shift.x + blockPoint.x) >= dims.x)
					||
					((block.shift.y + blockPoint.y) >= dims.y)
				;
			}
		);
	}

	bool overlapsWithOtherBlocks(const Block& block, const BoardState& boardState) {
		return
			((block.id != boardState.runner.id) && block.overlaps(boardState.runner))
			||
			std::any_of(
				begin(boardState.blocks),
				end(boardState.blocks),
				[&](const auto& other) { return (other.id != block.id) && block.overlaps(other); }
			)
		;
	}
}

bool validBlockPosition(
	const Block& block,
	const Point& dims,
	const BoardState& boardState,
	const std::set<Point> invalidPositions
) {
	return !detail::overlapsWithBorder(block, dims)
		&& !detail::overlapsWithInvalidSpaces(block, invalidPositions)
		&& !detail::overlapsWithOtherBlocks(block, boardState)
	;
}

