#pragma once

#include <string>
#include <set>
#include <sstream>

template <class T>
std::string toString(T val) {
	std::stringstream ss;
	ss << val;
	return ss.str();
}

enum Direction {
	Down,
	Right,
	Left,
	Up,
	Number_of_dirs
};

class Point {
public:
	int x;
	int y;
};

inline bool operator<( const Point& p1, const Point& p2) {
	if (p1.x == p2.x) {
		return (p1.y < p2.y);
	}
	return (p1.x < p2.x);
}
inline bool operator==(const Point& p1, const Point& p2) { return (p1.x == p2.x) && (p1.y == p2.y); }
inline bool operator!=(const Point& p1, const Point& p2) { return !(p1 == p2); }

class Block {
public:
	Point shift;
	std::set<Point> pointSet;
	std::string id;
	Block();
	Block(
		const Point& aShift,
		const std::set<Point>& aPointSet,
		const std::string& anId
	);

	bool overlaps(const Block& other) const;
	bool overlaps(const Point& otherPoint) const;
	Block move(const Direction& dir) const;
};

inline bool operator==(const Block& b1, const Block& b2) {
	return
		(b1.shift == b2.shift)
		&&
		(b1.id == b2.id)
		&&
		(b1.pointSet == b2.pointSet)
	;
}
inline bool operator!=(const Block& b1, const Block& b2) { return !(b1 == b2); }

