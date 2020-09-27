#pragma once

#include "puzzle.h"
#include <list>

void printSvgStyle(const Puzzle& puzzle, std::ostream& out);
void printSvg(const Point& point, std::ostream& out);
void printSvg(const Block& block, std::ostream& out);
void printSvg(const BoardState& boardState, std::ostream& out);
void printSvg(const Puzzle& puzzle, std::ostream& out);
void printSvg(const std::list<Puzzle>& puzzleList, std::string preffix, std::string suffix, std::ostream& out);

void printHtml(const std::list<Puzzle>& puzzleList, std::ostream& out);

void printText(const Puzzle& puzzle, std::ostream& out);
void printText(const std::list<Puzzle>& puzzleList, std::ostream& out);

Puzzle scanText(std::istream& in);

