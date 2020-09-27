#include "printer.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <regex>

std::map<int, const std::string> colorMap{
	{ 0, "pink" },
	{ 1, "aqua" },
	{ 2, "blueviolet" },
	{ 3, "darkblue" },
	{ 4, "darkcyan" },
	{ 5, "darkgoldenrod" },
	{ 6, "darkred" },
	{ 7, "darkmagenta" },
	{ 8, "darkorchid" },
	{ 9, "darkslategray" },
	{10, "firebrick" },
	{11, "green" },
	{12, "indigo" },
	{13, "seagreen" },
	{14, "teal" },
	{15, "yellowgreen" },
	{16, "yellow" },
};

void printSvgStyle(const Puzzle& puzzle, std::ostream& out) {
	out << " .dimensions { stroke : red; stroke-width : 0.1; }";
	out << " .blockRunner { fill : red; }";
	out << " .blockWall { fill : black; }";
	out << " .blockGoal { fill : powderblue; }";

	int iColor = -1;
	for (const Block& block : puzzle.boardState.blocks) {
		out << " .block" << block.id << " { fill : " << colorMap[++iColor] << "; }";
	}

	out << " .puzzle { fill : #202020; }";
}

void printSvg(const Point& point, std::ostream& out) {
	out << "<rect class='point' x='" << point.x << "' y='" << point.y << "' width='1' height='1'/>";
}

void printSvg(const Block& block, std::ostream& out) {
	out << "<g class='block block" << block.id << "' transform='translate(" << block.shift.x << " " << block.shift.y << ")'>";
	for (const Point& point : block.pointSet) {
		printSvg(point, out);
	}
	out << "</g>";
}

void printSvg(const BoardState& boardState, std::ostream& out) {
	printSvg({ boardState.runner.shift, boardState.runner.pointSet, "Runner" }, out);
	for (const Block& block : boardState.blocks) {
		printSvg(block, out);
	}
}

void printSvg(const Puzzle& puzzle, std::ostream& out) {
	out
		<< "<g class='puzzle'>"
		<< "<rect class='dimensions' x='0' y='0' width='" << puzzle.dimensions.x << "' height='" << puzzle.dimensions.y << "'/>"
	;

	Block forbiddenSpots;
	forbiddenSpots.shift = {0, 0};
	forbiddenSpots.pointSet = puzzle.forbiddenSpots;
	forbiddenSpots.id = "Wall";
	printSvg(forbiddenSpots, out);
	printSvg(Block(puzzle.goal, puzzle.boardState.runner.pointSet, "Goal"), out);
	printSvg(puzzle.boardState, out);
	out << "</g>";
}

void printSvg(const std::list<Puzzle>& puzzleList, std::string preffix, std::string suffix, std::ostream& out) {
	size_t i = 0;
	for (const Puzzle& puzzleStep : puzzleList) {
		const std::string pos(toString(++i));
		out << std::regex_replace(preffix, std::regex("\\{pos\\}"), pos, std::regex_constants::match_any);
		printSvg(puzzleStep, out);
		out << std::regex_replace(suffix,  std::regex("\\{pos\\}"), pos, std::regex_constants::match_any);
	}
}

void printHtml(const std::list<Puzzle>& puzzleList, std::ostream& out) {
	const Puzzle& firstPuzzle = *(puzzleList.begin());
	out << "<style>";
	printSvgStyle(firstPuzzle, out);
	out << " body { color : grey; background : black; }";
	out << "</style>\n<body>\n";
	printSvg(
		puzzleList,
		" {pos}<svg width='"
		+ toString((firstPuzzle.dimensions.x + 1) * 20) + "' height='"
		+ toString((firstPuzzle.dimensions.y + 1) * 20)
		+ "'><g transform='scale(20)'>",
		"</g></svg>\n",
		out
	);
	out << "</body>";
}

void printText(const Puzzle& puzzle, std::ostream& out) {

	//init layout
	std::vector<std::vector<char>> layout(
		puzzle.dimensions.x + 2,
		std::vector<char>(puzzle.dimensions.y + 2)
	);
	for (auto x = 1; x < puzzle.dimensions.x + 1; ++x) {
		for (auto y = 1; y < puzzle.dimensions.y + 1; ++y) {
			layout[x][y] = ' ';
		}
	}

	// Print border blocks
	for (auto x = 0; x < puzzle.dimensions.x + 2; ++x) {
		layout[x][0] = '#';
		layout[x][puzzle.dimensions.y + 1] = '#';
	}
	for (auto y = 1; y < puzzle.dimensions.y + 1; ++y) {
		layout[0][y] = '#';
		layout[puzzle.dimensions.x + 1][y] = '#';
	}

	// print content
	for (const auto &forbiddenPoint : puzzle.forbiddenSpots) {
		layout[forbiddenPoint.x + 1][forbiddenPoint.y + 1] = '#';
	}

	auto fillBlock = [&](const Block& block) {
		for (const auto& blockPoint : block.pointSet) {
			layout[block.shift.x + blockPoint.x + 1][block.shift.y + blockPoint.y + 1] = block.id[0];
		}
	};

	fillBlock({ puzzle.goal, puzzle.boardState.runner.pointSet, "^" });

	//fill runner block
	fillBlock(puzzle.boardState.runner);

	//fill other blocks
	for (const auto &contentBlock : puzzle.boardState.blocks) {
		fillBlock(contentBlock);
	}

	//prepare output
	out << puzzle.dimensions.x << " " << puzzle.dimensions.y << "\n";
	out << puzzle.goal.x << " " << puzzle.goal.y << "\n";
	for (auto y = 0; y < puzzle.dimensions.y + 2; ++y) {
		for (auto x = 0; x < puzzle.dimensions.x + 2; ++x) {
			out << layout[x][y];
		}
		out << "\n";
	}
}

void printText(const std::list<Puzzle>& puzzleList, std::ostream& out) {
	int pos = -1;
	for (const Puzzle& puzzleStep : puzzleList) {
		out << std::endl << "-------------- " << ++pos << "\n";
		printText(puzzleStep, out);
	}
}

Puzzle scanText(std::istream& in) {
	Point dimension;
	Point goal;
	in >> dimension.x >> dimension.y >> goal.x >> goal.y;
	//std::cout << dimension.x << "\n" << dimension.y << "\n" << goal.x << "\n" << goal.y << "\n";
	std::string line;
	std::getline(in, line);
	std::getline(in, line);

	std::map<char, std::set<Point>> objMap;
	for (auto y = 0; y < dimension.y; ++y) {
		std::getline(in, line);
		//std::cout << line << "\n";
		for (auto x = 0; x < dimension.x; ++x) {
			objMap[line[x + 1]].insert({ x, y });
		}
	}

	std::set<Point> runnerPointSet;
	std::string runnerId;
	std::set<Point> forbiddenSpots = {};
	std::vector<Block> blockList;
	for (auto objPair : objMap) {
		if ((' ' == objPair.first)) {
			//forget blanc place

		} else if ('^' == objPair.first) {
			//orget goal

		} else if ('@' == objPair.first) {
			runnerPointSet = objPair.second;
			runnerId = toString(objPair.first);

		} else if (line[0] == objPair.first) {
			forbiddenSpots = objPair.second;

		} else {
			blockList.push_back({
				{0, 0},
				objPair.second,
				toString(objPair.first)
			});

		}
	}

	return {
		dimension,
		goal,
		forbiddenSpots,
		{
			0,
			Block({0, 0}, runnerPointSet, runnerId),
			blockList
		}
	};
}

