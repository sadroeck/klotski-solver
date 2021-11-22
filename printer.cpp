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
	out <<
		" .dimensions { stroke : red; stroke-width : 0.01; }"
		" .block { fill-opacity : 0.93 }"
		" .blockRunner { fill : red; }"
		" .blockWall { fill : black; }"
		" .blockGoal { fill : url(#zebra-gradient) powderblue; }"
	;

	int iColor = 0;
	for (const Block& block : puzzle.boardState->blocks) {
		out << " .block" << block.id << " { fill : " << colorMap[iColor] << "; }";
		iColor = (iColor + 1) % colorMap.size();
	}

	out << " .puzzle { fill : #202020; }";
}

void printSvg(const Point& point, std::ostream& out) {
	out << "<rect class='point' x='" << point.x << "' y='" << point.y << "' width='1' height='1'/>";
}

void printSvg(const Block& block, const bool& displayId, std::ostream& out) {
	out << "\t<g";
	if (displayId) {
		out << " id='block" << block.id << "'";
	}
	out << " class='block block" << block.id << "' transform='translate(" << block.shift.x << " " << block.shift.y << ")'>";
	for (const Point& point : block.pointSet) {
		printSvg(point, out);
	}
	out << "</g>\n";
}

void printSvg(const BoardState& boardState, const bool& displayId, std::ostream& out) {
	printSvg({ boardState.runner.shift, boardState.runner.pointSet, "Runner" }, displayId, out);
	for (const Block& block : boardState.blocks) {
		printSvg(block, displayId, out);
	}
}

void printSvg(const Puzzle& puzzle, const bool& displayId, std::ostream& out) {
	out
		<< "<g class='puzzle'>"
		<< "<rect class='dimensions' x='0' y='0' width='" << puzzle.dimensions.x << "' height='" << puzzle.dimensions.y << "'/>\n"
	;

	Block forbiddenSpots;
	forbiddenSpots.shift = {0, 0};
	forbiddenSpots.pointSet = puzzle.forbiddenSpots;
	forbiddenSpots.id = "Wall";
	printSvg(forbiddenSpots, displayId, out);
	printSvg(Block(puzzle.goal, puzzle.boardState->runner.pointSet, "Goal"), displayId, out);
	printSvg(*(puzzle.boardState), displayId, out);
	out << "</g>";
}

void printSvg(
	const std::list<Puzzle>& puzzleList,
	const bool printAll,
	std::string preffix,
	std::string suffix,
	std::ostream& out
) {
	static auto replaceTag = [&](const std::string& text, const std::string& tag, const std::string& value) {
		return std::regex_replace(text, std::regex(tag), value, std::regex_constants::match_any);
	};
	static auto replaceTags = [&](const std::string& text, const std::string& step, const std::string& move) {
		return replaceTag(replaceTag(text, "\\{step\\}", step), "\\{move\\}", move);
	};

	size_t i = 0;
	const auto puzzleStepEnd = puzzleList.end();
	auto puzzleStepIt = puzzleList.begin();
	const Puzzle* previousPuzzleStep = &*puzzleStepIt;
	++puzzleStepIt;
	while (true) {
		if (
			printAll
			||
			(puzzleStepEnd == puzzleStepIt)
			||
			(previousPuzzleStep->boardState->moveNumber != puzzleStepIt->boardState->moveNumber)
		) {
			const std::string step(toString(i));
			const std::string moveNumber(toString(previousPuzzleStep->boardState->moveNumber));
			out << replaceTags(preffix, step, moveNumber);
			printSvg(*previousPuzzleStep, false, out);
			out << replaceTags(suffix, step, moveNumber);
		}
		if (puzzleStepEnd == puzzleStepIt) { break; }
		previousPuzzleStep = &*puzzleStepIt;
		++i;
		++puzzleStepIt;
	}
}

void printSvgAnimate(
	const std::string& id,
	const std::string& from,
	const std::string& to,
	const std::string& step,
	std::ostream& out
) {
	out << std::regex_replace(
		std::regex_replace(
			std::regex_replace(
				std::regex_replace(
					"<animateTransform"
					" xlink:href='#block{id}'"
					" attributeType='XML'"
					" attributeName='transform'"
					" type='translate'"
					" from='{from}'"
					" to='{to}'"
					" dur='1s'"
					" begin='{step}s'"
					"fill='freeze'"
					"/>\n",
					std::regex("\\{id\\}"),
					id
				),
				std::regex("\\{from\\}"),
				from
			),
			std::regex("\\{to\\}"),
			to
		),
		std::regex("\\{step\\}"),
		step
	);
}

void printSvgAnimate(const Block& b1, const Block& b2, const size_t& step, std::ostream& out) {
	if (b1 != b2) {
		printSvgAnimate(
			b1.id,
			toString(b1.shift.x) + ", " + toString(b1.shift.y),
			toString(b2.shift.x) + ", " + toString(b2.shift.y),
			toString(step),
			out
		);
	}
}

void printSvgAnimate(const Puzzle& puzzle1, const Puzzle& puzzle2, const size_t& step, std::ostream& out) {
	printSvgAnimate(
		{ puzzle1.boardState->runner.shift, puzzle1.boardState->runner.pointSet, "Runner" },
		{ puzzle2.boardState->runner.shift, puzzle2.boardState->runner.pointSet, "Runner" },
		step,
		out
	);
	for (size_t i = 0; i < puzzle1.boardState->blocks.size() ; ++i) {
		printSvgAnimate(puzzle1.boardState->blocks[i], puzzle2.boardState->blocks[i], step, out);
	}
}

void printSvgAnimate(const std::list<Puzzle>& puzzleList, std::ostream& out) {
	auto puzzle1It = puzzleList.begin();
	auto puzzle2It = ++(puzzleList.begin());
	size_t step = 0;
	while (puzzleList.end() != puzzle2It) {
		printSvgAnimate(*puzzle1It, *puzzle2It, ++step, out);
		++puzzle1It;
		++puzzle2It;
	}
}

void printHtml(
	const std::list<Puzzle>& puzzleList,
	const bool printAll,
	const size_t& animatedScale,
	const size_t& staticScale,
	std::ostream& out
) {
	Puzzle firstPuzzle = *(puzzleList.begin());
	out <<
		"<meta charset='UTF-8'>\n"
		"<style>"
	;
	printSvgStyle(firstPuzzle, out);
	out << " body { color : grey; background : black; }";
	out << "</style>\n<body>\n";
	if (animatedScale > 0) {
		out
			<< "<svg"
			<< " width='"  << ((firstPuzzle.dimensions.x + 1) * animatedScale) << "'"
			<< " height='" << ((firstPuzzle.dimensions.y + 1) * animatedScale) << "'"
			<< " xmlns:xlink='http://www.w3.org/1999/xlink'"
			<< ">\n"
			<< "<linearGradient id='zebra-gradient' x2='2' y2='1'>"
		;
		for (int i = 0; i < 100; i += 20) {
			out
				<< "<stop offset='" << (i +  0) << "%' stop-color='white'/><stop offset='" << (i + 10) << "%' stop-color='white'/>"
				<< "<stop offset='" << (i + 10) << "%' stop-color='black'/><stop offset='" << (i + 20) << "%' stop-color='black'/>"
			;
		}
		out
			<< "</linearGradient>\n"
			<< "<g transform='scale(" << animatedScale << ")'>"
		;
		printSvg(firstPuzzle, true, out);
		out << "</g>\n";
		printSvgAnimate(puzzleList, out);
		out << "</svg>\n";

	}
	if (staticScale > 0) {
		if (animatedScale > 0) {
			out << "<br/>\n";
		}
		printSvg(
			puzzleList,
			printAll,
			toString(" {move}<svg")
			+ " width='"  + toString((firstPuzzle.dimensions.x + 1) * staticScale) + "'"
			+ " height='" + toString((firstPuzzle.dimensions.y + 1) * staticScale) + "'"
			+ ">\n"
			+ "<g transform='scale(" + toString(staticScale) + ")'>",
			"</g>\n</svg>\n",
			out
		);
	}
	out << "</body>\n";
}

const char goalChar = '^';
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
	const char borderChar = '#';
	for (auto x = 0; x < puzzle.dimensions.x + 2; ++x) {
		layout[x][0] = borderChar;
		layout[x][puzzle.dimensions.y + 1] = borderChar;
	}
	for (auto y = 1; y < puzzle.dimensions.y + 1; ++y) {
		layout[0][y] = borderChar;
		layout[puzzle.dimensions.x + 1][y] = borderChar;
	}

	// print content
	for (const auto &forbiddenPoint : puzzle.forbiddenSpots) {
		layout[forbiddenPoint.x + 1][forbiddenPoint.y + 1] = borderChar;
	}

	auto fillBlock = [&](const Block& block) {
		for (const auto& blockPoint : block.pointSet) {
			layout[block.shift.x + blockPoint.x + 1][block.shift.y + blockPoint.y + 1] = block.id[0];
		}
	};

	fillBlock({ puzzle.goal, puzzle.boardState->runner.pointSet, toString(goalChar) });

	//fill runner block
	fillBlock(puzzle.boardState->runner);

	//fill other blocks
	for (const auto& contentBlock : puzzle.boardState->blocks) {
		fillBlock(contentBlock);
	}

	//prepare output
	out << puzzle.dimensions.x << " " << puzzle.dimensions.y << "\n";
	out << puzzle.goal.x << " " << puzzle.goal.y << " " << puzzle.boardState->runner.id << "\n";
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
		out << std::endl << "-------------- step:" << ++pos << " move:"<< puzzleStep.boardState->moveNumber << "\n";
		printText(puzzleStep, out);
	}
}

Puzzle scanText(std::istream& in) {
	Point dimension;
	Point goal;
	char runnerChar;
	in >> dimension.x >> dimension.y >> goal.x >> goal.y >> runnerChar;
	//std::cout << dimension.x << "\n" << dimension.y << "\n" << goal.x << "\n" << goal.y << "\n" << runnerChar << '\n';
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

		} else if (goalChar == objPair.first) {
			//forget goal

		} else if (runnerChar == objPair.first) {
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
		std::make_shared<BoardState>(BoardState{
			0,
			Block({0, 0}, runnerPointSet, runnerId),
			blockList
		})
	};
}

