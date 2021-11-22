#pragma once

#include "puzzle.h"
#include <list>

void printSvgStyle(const Puzzle& puzzle, std::ostream& out);
void printSvg(const Point& point, std::ostream& out);
void printSvg(const Block& block, const bool& displayId, std::ostream& out);
void printSvg(const BoardState& boardState, const bool& displayId, std::ostream& out);
void printSvg(const Puzzle& puzzle, const bool& displayId, std::ostream& out);
void printSvg(
	const std::list<Puzzle>& puzzleList,
	const bool printAll,
	std::string preffix,
	std::string suffix,
	std::ostream& out
);

void printSvgAnimate(
	const std::string& id,
	const std::string& from,
	const std::string& to,
	const std::string& step,
	std::ostream& out
);
void printSvgAnimate(const Block& b1, const Block& b2, const size_t& step, std::ostream& out);
void printSvgAnimate(const Puzzle& puzzle1, const Puzzle& puzzle2, const size_t& step, std::ostream& out);
void printSvgAnimate(const std::list<Puzzle>& puzzleList, std::ostream& out);

void printHtml(
	const std::list<Puzzle>& puzzleList,
	const bool printAll,
	const size_t& animatedScale,
	const size_t& staticScale,
	std::ostream& out
);

void printText(const Puzzle& puzzle, std::ostream& out);
void printText(const std::list<Puzzle>& puzzleList, std::ostream& out);

Puzzle scanText(std::istream& in);

