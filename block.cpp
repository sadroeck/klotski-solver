#include "block.h"

#include <algorithm>
#include <stdexcept>

Block::Block() { }
Block::Block(
	const Point& aShift,
	const std::set<Point>& aPointSet,
	const std::string& anId
) {
	auto aPointSetIt = aPointSet.begin();
	const auto aPointSetItEnd = aPointSet.end();
	if (aPointSetIt == aPointSetItEnd) {
		//empty point list
		shift = aShift;

	} else {
		int xMin = aPointSetIt->x;
		int yMin = aPointSetIt->y;
		++aPointSetIt;
		for (; aPointSetIt != aPointSetItEnd; ++aPointSetIt) {
			if (xMin > aPointSetIt->x) { xMin = aPointSetIt->x; }
			if (yMin > aPointSetIt->y) { yMin = aPointSetIt->y; }
		}
		shift.x = aShift.x + xMin;
		shift.y = aShift.y + yMin;

		for (auto& p : aPointSet) {
			pointSet.insert({ p.x - xMin, p.y - yMin });
		}
	}

	id = anId;
}

bool Block::overlaps(const Point& otherPoint) const {
	for (auto& thisPoint : this->pointSet) {
		if (
			((this->shift.x + thisPoint.x) == (otherPoint.x))
			&&
			((this->shift.y + thisPoint.y) == (otherPoint.y))
		) {
			return true;
		}
	}
	return false;
}

bool Block::overlaps(const Block& other) const {
	for (auto& thisPoint : this->pointSet) {
		for (auto& otherPoint : other.pointSet) {
			if (
				((this->shift.x + thisPoint.x) == (other.shift.x + otherPoint.x))
				&&
				((this->shift.y + thisPoint.y) == (other.shift.y + otherPoint.y))
			) {
				return true;
			}
		}
	}
	return false;
}

Block Block::move(const Direction& dir) const {
	switch (dir) {
		case Up:    return Block({ shift.x    , shift.y - 1 }, pointSet, id);
		case Down:  return Block({ shift.x    , shift.y + 1 }, pointSet, id);
		case Left:  return Block({ shift.x - 1, shift.y     }, pointSet, id);
		case Right: return Block({ shift.x + 1, shift.y     }, pointSet, id);
		default:
			throw std::runtime_error("Unknown direction " + static_cast<int>(dir));
	}
}

