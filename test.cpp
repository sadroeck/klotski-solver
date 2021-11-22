#include "solver.h"
#include "printer.h"
#include "BoardHasher.h"
#include "MoveDiscovery.h"
#include "MoveValidation.h"

#include <cassert>
#include <iostream>
#include <set>


namespace {
	auto compareString = [&](const std::string& s1, const std::string& s2) {
		if (s1 != s2) {
			std::cout << s1 << "\n!=\n" << s2 << "\n";
			std::cout << "(len1:" << s1.size() << ")\n(len2:" << s2.size() << ")\n";
			for (size_t i = 0; i < s1.size(); ++i) {
				if (s1[i] != s2[i]) {
					std::cout << "(first diff at position " << i << ") ('" << s1[i] << "' != '" << s2[i] << "')\n";
					//std::cout << "(first diff at position " << i << ") ('" << s1.substr(i - 10, 20) << "' != '" << s2.substr(i - 10, 20) << "')\n";
					break;
				}
			}
		}
		return s1 == s2;
	};

	// Standard klotski puzzle
	// print for sample ascii layout
	const Puzzle largePuzzle {
		{ 4, 6 }, // dims
		{ 1, 4 }, // goal
		{ // invalid spaces
			{ 0, 5 },
			{ 3, 5 },
		},
		std::make_shared<BoardState>(BoardState{
			0, // no moves made,
			Block({ 1, 0 }, { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } }, "@"), // runner
			{ // blocks
				Block({ 0, 0 }, { { 0, 0 }, { 0, 1 } }, "A"),
				Block({ 0, 2 }, { { 0, 0 }, { 0, 1 } }, "B"),
				Block({ 1, 2 }, { { 0, 0 }, { 1, 0 } }, "C"),
				Block({ 1, 3 }, { { 0, 0 }           }, "D"),
				Block({ 2, 3 }, { { 0, 0 }           }, "E"),
				Block({ 3, 0 }, { { 0, 0 }, { 0, 1 } }, "F"),
				Block({ 3, 2 }, { { 0, 0 }, { 0, 1 } }, "G"),
				Block({ 0, 4 }, { { 0, 0 }           }, "H"),
				Block({ 3, 4 }, { { 0, 0 }           }, "I")
			}
		})
	};

	// Empty 3x3 block
	// Runner 1x1 at the origin
	// Single 1x1 block in the middle of the board
	const Puzzle emptyPuzzle {
		{ 3, 3 }, // dims
		{ 2, 2 }, // goal
		{
			{ 1, 1 } // single hindrance in the middle
		}, // empty spaces
		std::make_shared<BoardState>(BoardState{
			0, // no moves made,
			Block({ 0, 0 }, { { 0, 0 } }, "@"), // runner at origin
			{} // no blocks
		})
	};

	const Puzzle tinyPuzzle
	{
		{ 3, 3 }, // dims
		{ 1, 1 }, // goal
		{}, // empty spaces
		std::make_shared<BoardState>(BoardState{
			0, // no moves made,
			Block({ 0, 0 }, { { 0, 0 } }, "@"), // runner
			{
				Block({1, 1 }, { { 0, 0 } }, "A") // Single block on the goal
			}
		})
	};

	// Empty 3x3 block
	// Runner 1x1 at the origin
	// Single 1x1 block in the middle of the board
	const Puzzle smallPuzzle
	{
		{ 3, 3 }, // dims
		{ 2, 2 }, // goal
		{
			{ 1, 1 }
		}, // empty spaces
		std::make_shared<BoardState>(BoardState{
			0, // no moves made,
			Block({ 0, 0 }, { { 0, 0 } }, "@"), // runner at origin
			{ // 2 blocks surrounding origin
				Block({ 1, 0 }, { { 0, 0 } }, "A"),
				Block({ 0, 1 }, { { 0, 0 } }, "B")
			}
		})
	};

	// Tetris klotski puzzle
	// print for sample ascii layout
	const Puzzle tetrisPuzzle {
		{ 5, 6 }, // dims
		{ 0, 0 }, // goal
		{ // invalid spaces
			{ 0, 5 },
			{ 4, 5 },
		},
		std::make_shared<BoardState>(BoardState{
			0, // no moves made,
			Block({ 3, 2 }, { { 0, 0 }, { 0, 2 }, { 1, 0 }, { 1, 1 }, { 1, 2 } }, "@"), // runner
			{ // blocks
				Block({ 2, 1 }, { { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1, 0 }, { 1, 2 } }, "A")
			}
		})
	};

}

void testPuzzles() {
	auto testPuzzle = [&](const auto& puzzle) {
		auto puzzleValidation = puzzle.validate();
		if (valid != puzzleValidation) {
			printText(puzzle, std::cout);
			std::cout << Puzzle::validationString(puzzleValidation) << std::endl;
			return false;
		}
		return true;
	};

	assert(testPuzzle(emptyPuzzle));
	assert(testPuzzle(tinyPuzzle));
	assert(testPuzzle(smallPuzzle));
	assert(testPuzzle(largePuzzle));
	assert(testPuzzle(tetrisPuzzle));
}

void testPoint() {
	Point p1{2, 5};
	Point p2{2, 4};
	Point p3{2, 5};
	assert(p1 != p2);
	assert(p1 == p3);

	std::set<Point> pointSet = {
		{  0,  0 },
		{  1,  5 },
		{  0,  0 },
		{ -1,  5 },
		{  0,  0 },
		{  1, -5 },
		{  1,  4 },
	};
	std::stringstream orderResult;
	for (const Point& p : pointSet) {
		orderResult << p.x << ", " << p.y << "\n";
	}
	const std::string orderExpected =
		"-1, 5\n"
		"0, 0\n"
		"1, -5\n"
		"1, 4\n"
		"1, 5\n"
	;
	assert(compareString(orderResult.str(), orderExpected));
}

void testBlocks() {
	{
		//overlaps Block
		Block singleLeft({    1, 1 }, { { 0, 0 } }, "1");
		Block singleRight({   3, 1 }, { { 0, 0 } }, "2");
		Block singleMiddle({  2, 1 }, { { 0, 0 } }, "3");
		Block doubleLeft({    0, 0 }, { { 0, 0 }, { 1, 0 } }, "4");
		Block doubleRight({   1, 0 }, { { 0, 0 }, { 1, 0 } }, "5");
		Block somewhereElse({ 2, 2 }, { { 0, 0 } }, "6");

		assert(!singleLeft.overlaps(singleRight));
		assert(!singleLeft.overlaps(singleMiddle));
		assert(doubleLeft.overlaps(doubleRight) && "Block should overlap in the middle");
		assert(doubleRight.overlaps(doubleLeft) && "Block should overlap in the middle");
		assert(!doubleLeft.overlaps(somewhereElse) && "Blocks should not overlap");
		assert(!doubleRight.overlaps(somewhereElse) && "Blocks should not overlap");
	}

	{
		//overlaps Point
		Block b({ 1, 1 }, { { 0, 1 }, { 1, 0 } }, "x");

		std::stringstream strResult;
		for (int y = 0; y < 4; ++y) {
			for (int x = 0; x < 4; ++x) {
				strResult << x << ", " << y << " : " << ((b.overlaps(Point{ x, y })) ? "true" : "false") << "\n";
			}
		}
		assert(compareString(
			strResult.str(),
			"0, 0 : false\n"
			"1, 0 : false\n"
			"2, 0 : false\n"
			"3, 0 : false\n"

			"0, 1 : false\n"
			"1, 1 : false\n"
			"2, 1 : true\n"
			"3, 1 : false\n"

			"0, 2 : false\n"
			"1, 2 : true\n"
			"2, 2 : false\n"
			"3, 2 : false\n"

			"0, 3 : false\n"
			"1, 3 : false\n"
			"2, 3 : false\n"
			"3, 3 : false\n"
		));
	}

	{
		std::vector<Block> shapeList = {
			Block({ 0, 0 }, { { 0, 0 } }, "1"),
			Block({ 1, 5 }, { { 0, 0 } }, "2"),
			Block({ 0, 0 }, { { 0, 0 }, { 1, 1 } }, "3"),
			Block({ 0, 0 }, { { 1, 1 }, { 0, 0 } }, "4"),
		};

		std::stringstream shapeComareResult;
		for (size_t i = 0; i < shapeList.size(); ++i) {
			for (size_t j = 0; j < shapeList.size(); ++j) {
				shapeComareResult << i << ", " << j << " : " << ((shapeList[i].pointSet == shapeList[j].pointSet) ? "true" : "false") << "\n";
			}
		}
		assert(compareString(
			shapeComareResult.str(),
			"0, 0 : true\n"
			"0, 1 : true\n"
			"0, 2 : false\n"
			"0, 3 : false\n"

			"1, 0 : true\n"
			"1, 1 : true\n"
			"1, 2 : false\n"
			"1, 3 : false\n"

			"2, 0 : false\n"
			"2, 1 : false\n"
			"2, 2 : true\n"
			"2, 3 : true\n"

			"3, 0 : false\n"
			"3, 1 : false\n"
			"3, 2 : true\n"
			"3, 3 : true\n"
		));
	}
}

void testPrintScan() {
	auto checkPrintScan = [&](const Puzzle& p, const std::string& expectedText) {
		//print text
		std::stringstream strResult;
		printText(p, strResult);
		if (!compareString(strResult.str(), expectedText)) {
			return false;
		}

		//scan
		Puzzle scanResult(scanText(strResult));

		if (scanResult != p) {
			printText(p, std::cout);
			std::cout << "puzzle scan differs\n";
			printText(scanResult, std::cout);
			return false;
		}

		return true;
	};

	assert(checkPrintScan(
		tetrisPuzzle,
		"5 6\n"
		"0 0 @\n"
		"#######\n"
		"#^^   #\n"
		"# ^AA #\n"
		"#^^A@@#\n"
		"#  AA@#\n"
		"#   @@#\n"
		"##   ##\n"
		"#######\n"
	));

	assert(checkPrintScan(
		largePuzzle,
		"4 6\n"
		"1 4 @\n"
		"######\n"
		"#A@@F#\n"
		"#A@@F#\n"
		"#BCCG#\n"
		"#BDEG#\n"
		"#H^^I#\n"
		"##^^##\n"
		"######\n"
	));

	{
		//print svg style
		std::stringstream strResult;
		printSvgStyle(
			{
				{ 20, 20 }, // dims
				{ 19, 19 }, // goal
				{ // invalid spaces
				},
				std::make_shared<BoardState>(BoardState{
					0, // no moves made,
					Block({ 0, 0 }, { { 0, 0 }, }, "@"), // runner
					{ // blocks
						Block({  0,  0 }, { {  0,  0 }, }, "A"),
						Block({  0,  0 }, { {  0,  0 }, }, "B"),
						Block({  0,  0 }, { {  0,  0 }, }, "C"),
						Block({  0,  0 }, { {  0,  0 }, }, "D"),
						Block({  0,  0 }, { {  0,  0 }, }, "E"),
						Block({  0,  0 }, { {  0,  0 }, }, "F"),
						Block({  0,  0 }, { {  0,  0 }, }, "G"),
						Block({  0,  0 }, { {  0,  0 }, }, "H"),
						Block({  0,  0 }, { {  0,  0 }, }, "I"),
						Block({  0,  0 }, { {  0,  0 }, }, "J"),
						Block({  0,  0 }, { {  0,  0 }, }, "K"),
						Block({  0,  0 }, { {  0,  0 }, }, "L"),
						Block({  0,  0 }, { {  0,  0 }, }, "M"),
						Block({  0,  0 }, { {  0,  0 }, }, "N"),
						Block({  0,  0 }, { {  0,  0 }, }, "O"),
						Block({  0,  0 }, { {  0,  0 }, }, "P"),
						Block({  0,  0 }, { {  0,  0 }, }, "Q"),
						Block({  0,  0 }, { {  0,  0 }, }, "R"),
						Block({  0,  0 }, { {  0,  0 }, }, "S"),
						Block({  0,  0 }, { {  0,  0 }, }, "T"),
						Block({  0,  0 }, { {  0,  0 }, }, "U"),
						Block({  0,  0 }, { {  0,  0 }, }, "V"),
						Block({  0,  0 }, { {  0,  0 }, }, "W"),
						Block({  0,  0 }, { {  0,  0 }, }, "X"),
						Block({  0,  0 }, { {  0,  0 }, }, "Y"),
						Block({  0,  0 }, { {  0,  0 }, }, "Z"),
					}
				})
			},
			strResult
		);
		assert(compareString(
			strResult.str(),
			" .dimensions { stroke : red; stroke-width : 0.01; }"
			" .block { fill-opacity : 0.93 }"
			" .blockRunner { fill : red; }"
			" .blockWall { fill : black; }"
			" .blockGoal { fill : url(#zebra-gradient) powderblue; }"
			" .blockA { fill : pink; }"
			" .blockB { fill : aqua; }"
			" .blockC { fill : blueviolet; }"
			" .blockD { fill : darkblue; }"
			" .blockE { fill : darkcyan; }"
			" .blockF { fill : darkgoldenrod; }"
			" .blockG { fill : darkred; }"
			" .blockH { fill : darkmagenta; }"
			" .blockI { fill : darkorchid; }"
			" .blockJ { fill : darkslategray; }"
			" .blockK { fill : firebrick; }"
			" .blockL { fill : green; }"
			" .blockM { fill : indigo; }"
			" .blockN { fill : seagreen; }"
			" .blockO { fill : teal; }"
			" .blockP { fill : yellowgreen; }"
			" .blockQ { fill : yellow; }"
			" .blockR { fill : pink; }"
			" .blockS { fill : aqua; }"
			" .blockT { fill : blueviolet; }"
			" .blockU { fill : darkblue; }"
			" .blockV { fill : darkcyan; }"
			" .blockW { fill : darkgoldenrod; }"
			" .blockX { fill : darkred; }"
			" .blockY { fill : darkmagenta; }"
			" .blockZ { fill : darkorchid; }"
			" .puzzle { fill : #202020; }"
		));
	}

	{
		//print svg Point
		std::stringstream strResult;
		printSvg(Point{2, 5}, strResult);
		assert(compareString(strResult.str(), "<rect class='point' x='2' y='5' width='1' height='1'/>"));
	}

	{
		//print svg Block
		std::stringstream strResult;
		printSvg(Block({ 1, 2 }, { { 0, 0 }, { 0, 1 }, { 1, 0 } }, "A"), true,  strResult);
		strResult << "\n---\n";
		printSvg(Block({ 2, 3 }, { { 4, 5 }, { 6, 7 }, { 8, 9 } }, "A"), false, strResult);
		assert(compareString(strResult.str(),
			"\t<g id='blockA' class='block blockA' transform='translate(1 2)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"</g>\n"
			"\n---\n"
			"\t<g class='block blockA' transform='translate(6 8)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='2' y='2' width='1' height='1'/>"
			"<rect class='point' x='4' y='4' width='1' height='1'/>"
			"</g>\n"
		));
	}

	{
		//print svg BoardState
		std::stringstream strResult;
		printSvg(*(tetrisPuzzle.boardState), true, strResult);
		strResult << "\n---\n";
		printSvg(*(tetrisPuzzle.boardState), false, strResult);
		assert(compareString(strResult.str(),
			"\t<g id='blockRunner' class='block blockRunner' transform='translate(3 2)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g id='blockA' class='block blockA' transform='translate(2 1)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='1' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\n---\n"
			"\t<g class='block blockRunner' transform='translate(3 2)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockA' transform='translate(2 1)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='1' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
		));
	}

	{
		//print svg Puzzle
		std::stringstream strResult;
		printSvg(tetrisPuzzle, true, strResult);
		strResult << "\n---\n";
		printSvg(tetrisPuzzle, false, strResult);
		assert(compareString(strResult.str(),
			"<g class='puzzle'>"
			"<rect class='dimensions' x='0' y='0' width='5' height='6'/>\n"
			"\t<g id='blockWall' class='block blockWall' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='5' width='1' height='1'/>"
			"<rect class='point' x='4' y='5' width='1' height='1'/>"
			"</g>\n"
			"\t<g id='blockGoal' class='block blockGoal' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g id='blockRunner' class='block blockRunner' transform='translate(3 2)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g id='blockA' class='block blockA' transform='translate(2 1)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='1' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"</g>"
			"\n---\n"
			"<g class='puzzle'>"
			"<rect class='dimensions' x='0' y='0' width='5' height='6'/>\n"
			"\t<g class='block blockWall' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='5' width='1' height='1'/>"
			"<rect class='point' x='4' y='5' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockGoal' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockRunner' transform='translate(3 2)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockA' transform='translate(2 1)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='1' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"</g>"
		));
	}

	{
		//print svg Puzzle list
		std::stringstream strResult;
		printSvg(std::list<Puzzle>{ tetrisPuzzle, tetrisPuzzle }, true, "av", "ap", strResult);
		strResult << "\n---\n";
		printSvg(std::list<Puzzle>{ tetrisPuzzle, tetrisPuzzle }, false, "av", "ap", strResult);
		assert(compareString(strResult.str(),
			"av<g class='puzzle'>"
			"<rect class='dimensions' x='0' y='0' width='5' height='6'/>\n"
			"\t<g class='block blockWall' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='5' width='1' height='1'/>"
			"<rect class='point' x='4' y='5' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockGoal' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockRunner' transform='translate(3 2)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockA' transform='translate(2 1)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='1' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"</g>apav"
			"<g class='puzzle'>"
			"<rect class='dimensions' x='0' y='0' width='5' height='6'/>\n"
			"\t<g class='block blockWall' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='5' width='1' height='1'/>"
			"<rect class='point' x='4' y='5' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockGoal' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockRunner' transform='translate(3 2)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockA' transform='translate(2 1)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='1' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"</g>ap"
			"\n---\n"
			"av<g class='puzzle'>"
			"<rect class='dimensions' x='0' y='0' width='5' height='6'/>\n"
			"\t<g class='block blockWall' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='5' width='1' height='1'/>"
			"<rect class='point' x='4' y='5' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockGoal' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockRunner' transform='translate(3 2)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockA' transform='translate(2 1)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='1' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"</g>ap"
		));
	}

	{
		//print svg animate tag
		std::stringstream strResult;
		printSvgAnimate("anId", "aFrom", "aTo", "aStep", strResult);
		assert(compareString(strResult.str(),
			"<animateTransform"
			" xlink:href='#blockanId'"
			" attributeType='XML'"
			" attributeName='transform'"
			" type='translate'"
			" from='aFrom'"
			" to='aTo'"
			" dur='1s'"
			" begin='aSteps'"
			"fill='freeze'"
			"/>\n"
		));
	}
	{
		//print svg animate Block
		std::stringstream strResult;
		printSvgAnimate(
			tetrisPuzzle.boardState->runner,
			tetrisPuzzle.boardState->runner,
			4,
			strResult
		);
		strResult << "\n---\n";
		printSvgAnimate(
			{ { 0, 1 }, tetrisPuzzle.boardState->runner.pointSet, "anId" },
			{ { 2, 3 }, tetrisPuzzle.boardState->runner.pointSet, "anId" },
			5,
			strResult
		);
		assert(compareString(strResult.str(),
			"\n---\n"
			"<animateTransform"
			" xlink:href='#blockanId'"
			" attributeType='XML'"
			" attributeName='transform'"
			" type='translate'"
			" from='0, 1'"
			" to='2, 3'"
			" dur='1s'"
			" begin='5s'"
			"fill='freeze'"
			"/>\n"
		));
	}
	{
		//print svg animate Puzzle
		std::stringstream strResult;
		printSvgAnimate(
			smallPuzzle,
			smallPuzzle,
			6,
			strResult
		);
		strResult << "\n---\n";
		printSvgAnimate(
			smallPuzzle,
			{ smallPuzzle.dimensions, smallPuzzle.goal, smallPuzzle.forbiddenSpots, std::make_shared<BoardState>(BoardState{ smallPuzzle.boardState->moveNumber, { { 0, 0 }, smallPuzzle.boardState->runner.pointSet, "anOtherId" }, { Block({ 1, 0 }, { { 0, 0 } }, "A"),	Block({ 0, 0 }, { { 0, 0 } }, "B") } }) },
			7,
			strResult
		);
		assert(compareString(strResult.str(),
			"\n---\n"
			"<animateTransform"
			" xlink:href='#blockB'"
			" attributeType='XML'"
			" attributeName='transform'"
			" type='translate'"
			" from='0, 1'"
			" to='0, 0'"
			" dur='1s'"
			" begin='7s'"
			"fill='freeze'"
			"/>\n"
		));
	}
	{
		//print svg animate Puzzle list
		std::stringstream strResult;
		printSvgAnimate(
			{
				smallPuzzle,
				{ smallPuzzle.dimensions, smallPuzzle.goal, smallPuzzle.forbiddenSpots, std::make_shared<BoardState>(BoardState{ smallPuzzle.boardState->moveNumber, { { 0, 0 }, smallPuzzle.boardState->runner.pointSet, "anOtherId" }, { Block({ 1, 0 }, { { 0, 0 } }, "A"),	Block({ 0, 0 }, { { 0, 0 } }, "B") } }) },
				{ smallPuzzle.dimensions, smallPuzzle.goal, smallPuzzle.forbiddenSpots, std::make_shared<BoardState>(BoardState{ smallPuzzle.boardState->moveNumber, { { 0, 0 }, smallPuzzle.boardState->runner.pointSet, "anOtherId" }, { Block({ 1, 1 }, { { 0, 0 } }, "A"),	Block({ 0, 0 }, { { 0, 0 } }, "B") } }) },
			},
			strResult
		);
		assert(compareString(strResult.str(),
			"<animateTransform"
			" xlink:href='#blockB'"
			" attributeType='XML'"
			" attributeName='transform'"
			" type='translate'"
			" from='0, 1'"
			" to='0, 0'"
			" dur='1s'"
			" begin='1s'"
			"fill='freeze'"
			"/>\n"
			"<animateTransform"
			" xlink:href='#blockA'"
			" attributeType='XML'"
			" attributeName='transform'"
			" type='translate'"
			" from='1, 0'"
			" to='1, 1'"
			" dur='1s'"
			" begin='2s'"
			"fill='freeze'"
			"/>\n"
		));
	}

	{
		//print html Puzzle list
		const Puzzle tetrisPuzzle2 {
			tetrisPuzzle.dimensions,
			tetrisPuzzle.goal,
			tetrisPuzzle.forbiddenSpots,
			std::make_shared<BoardState>(BoardState{
				1,
				Block({ 0, 0 }, { { 0, 0 }, { 0, 2 }, { 1, 0 }, { 1, 1 }, { 1, 2 } }, "@"), // runner
				{ // blocks
					Block({ 3, 1 }, { { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1, 0 }, { 1, 2 } }, "A")
				}
			})
		};

		std::stringstream strResult;
		printHtml(std::list<Puzzle>{ tetrisPuzzle, tetrisPuzzle2}, true, 0,  20, strResult);
		strResult << "\n---\n";
		printHtml(std::list<Puzzle>{ tetrisPuzzle, tetrisPuzzle2}, true, 100, 0, strResult);
		assert(compareString(strResult.str(),
			"<meta charset='UTF-8'>\n"
			"<style>"
			" .dimensions { stroke : red; stroke-width : 0.01; }"
			" .block { fill-opacity : 0.93 }"
			" .blockRunner { fill : red; }"
			" .blockWall { fill : black; }"
			" .blockGoal { fill : url(#zebra-gradient) powderblue; }"
			" .blockA { fill : pink; }"
			" .puzzle { fill : #202020; }"
			" body { color : grey; background : black; }"
			"</style>\n"
			"<body>\n"
			" 0<svg width='120' height='140'>\n<g transform='scale(20)'><g class='puzzle'>"
			"<rect class='dimensions' x='0' y='0' width='5' height='6'/>\n"
			"\t<g class='block blockWall' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='5' width='1' height='1'/>"
			"<rect class='point' x='4' y='5' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockGoal' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockRunner' transform='translate(3 2)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockA' transform='translate(2 1)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='1' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"</g></g>\n</svg>\n"
			" 1<svg width='120' height='140'>\n<g transform='scale(20)'>"
			"<g class='puzzle'>"
			"<rect class='dimensions' x='0' y='0' width='5' height='6'/>\n"
			"\t<g class='block blockWall' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='5' width='1' height='1'/>"
			"<rect class='point' x='4' y='5' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockGoal' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockRunner' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g class='block blockA' transform='translate(3 1)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='1' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"</g></g>\n</svg>\n"
			"</body>\n"
			"\n---\n"
			"<meta charset='UTF-8'>\n"
			"<style>"
			" .dimensions { stroke : red; stroke-width : 0.01; }"
			" .block { fill-opacity : 0.93 }"
			" .blockRunner { fill : red; }"
			" .blockWall { fill : black; }"
			" .blockGoal { fill : url(#zebra-gradient) powderblue; }"
			" .blockA { fill : pink; }"
			" .puzzle { fill : #202020; }"
			" body { color : grey; background : black; }"
			"</style>\n"
			"<body>\n"
			"<svg width='600' height='700' xmlns:xlink='http://www.w3.org/1999/xlink'>\n"
			"<linearGradient id='zebra-gradient' x2='2' y2='1'><stop offset='0%' stop-color='white'/><stop offset='10%' stop-color='white'/><stop offset='10%' stop-color='black'/><stop offset='20%' stop-color='black'/><stop offset='20%' stop-color='white'/><stop offset='30%' stop-color='white'/><stop offset='30%' stop-color='black'/><stop offset='40%' stop-color='black'/><stop offset='40%' stop-color='white'/><stop offset='50%' stop-color='white'/><stop offset='50%' stop-color='black'/><stop offset='60%' stop-color='black'/><stop offset='60%' stop-color='white'/><stop offset='70%' stop-color='white'/><stop offset='70%' stop-color='black'/><stop offset='80%' stop-color='black'/><stop offset='80%' stop-color='white'/><stop offset='90%' stop-color='white'/><stop offset='90%' stop-color='black'/><stop offset='100%' stop-color='black'/></linearGradient>\n"
			"<g transform='scale(100)'><g class='puzzle'><rect class='dimensions' x='0' y='0' width='5' height='6'/>\n"
			"\t<g id='blockWall' class='block blockWall' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='5' width='1' height='1'/>"
			"<rect class='point' x='4' y='5' width='1' height='1'/>"
			"</g>\n"
			"\t<g id='blockGoal' class='block blockGoal' transform='translate(0 0)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g id='blockRunner' class='block blockRunner' transform='translate(3 2)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='1' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"\t<g id='blockA' class='block blockA' transform='translate(2 1)'>"
			"<rect class='point' x='0' y='0' width='1' height='1'/>"
			"<rect class='point' x='0' y='1' width='1' height='1'/>"
			"<rect class='point' x='0' y='2' width='1' height='1'/>"
			"<rect class='point' x='1' y='0' width='1' height='1'/>"
			"<rect class='point' x='1' y='2' width='1' height='1'/>"
			"</g>\n"
			"</g></g>\n"
			"<animateTransform"
			" xlink:href='#blockRunner'"
			" attributeType='XML'"
			" attributeName='transform'"
			" type='translate'"
			" from='3, 2'"
			" to='0, 0'"
			" dur='1s'"
			" begin='1s'"
			"fill='freeze'"
			"/>\n"
			"<animateTransform"
			" xlink:href='#blockA'"
			" attributeType='XML'"
			" attributeName='transform'"
			" type='translate'"
			" from='2, 1'"
			" to='3, 1'"
			" dur='1s'"
			" begin='1s'"
			"fill='freeze'"
			"/>\n"
			"</svg>\n"
			"</body>\n"
		));
	}

}


void testMoveValidation() {
	// Test if class can be constructed
	//MoveValidation moveValidation{};

	// Test that all blocks do not overlap with their current state on the board
	for (const auto &block : largePuzzle.boardState->blocks) {
		assert(
			validBlockPosition(block, largePuzzle.dimensions, *(largePuzzle.boardState), largePuzzle.forbiddenSpots)
			&& "A block should never overlap itself"
		);

		auto blockAtSamePosition = block;
		blockAtSamePosition.id = "Another Block";
		assert(
			!validBlockPosition(blockAtSamePosition, largePuzzle.dimensions, *(largePuzzle.boardState), largePuzzle.forbiddenSpots)
			&& "2 blocks at the same position should always overlap"
		);
	}

	// Test all block pisition
	//printText(emptyPuzzle, std::cout);
	assert(!validBlockPosition(Block({ 0, 0 }, { { 0, 0 } }, "someBlock"), emptyPuzzle.dimensions, *(emptyPuzzle.boardState), emptyPuzzle.forbiddenSpots));
	assert( validBlockPosition(Block({ 1, 0 }, { { 0, 0 } }, "someBlock"), emptyPuzzle.dimensions, *(emptyPuzzle.boardState), emptyPuzzle.forbiddenSpots));
	assert( validBlockPosition(Block({ 2, 0 }, { { 0, 0 } }, "someBlock"), emptyPuzzle.dimensions, *(emptyPuzzle.boardState), emptyPuzzle.forbiddenSpots));
	assert( validBlockPosition(Block({ 0, 1 }, { { 0, 0 } }, "someBlock"), emptyPuzzle.dimensions, *(emptyPuzzle.boardState), emptyPuzzle.forbiddenSpots));
	assert(!validBlockPosition(Block({ 1, 1 }, { { 0, 0 } }, "someBlock"), emptyPuzzle.dimensions, *(emptyPuzzle.boardState), emptyPuzzle.forbiddenSpots));
	assert( validBlockPosition(Block({ 2, 1 }, { { 0, 0 } }, "someBlock"), emptyPuzzle.dimensions, *(emptyPuzzle.boardState), emptyPuzzle.forbiddenSpots));
	assert( validBlockPosition(Block({ 0, 2 }, { { 0, 0 } }, "someBlock"), emptyPuzzle.dimensions, *(emptyPuzzle.boardState), emptyPuzzle.forbiddenSpots));
	assert( validBlockPosition(Block({ 1, 2 }, { { 0, 0 } }, "someBlock"), emptyPuzzle.dimensions, *(emptyPuzzle.boardState), emptyPuzzle.forbiddenSpots));
	assert( validBlockPosition(Block({ 2, 2 }, { { 0, 0 } }, "someBlock"), emptyPuzzle.dimensions, *(emptyPuzzle.boardState), emptyPuzzle.forbiddenSpots));
}

void testMoveDiscovery() {
	MoveRunnerFirst moveDiscovery;
	{
		const int runnerMoves = 2; // Down + Right
		const int blockMoves = 4; // Up + Down + Right + Left
		const auto newMoves = moveDiscovery.gatherMoves(
			tinyPuzzle.dimensions,
			tinyPuzzle.boardState,
			tinyPuzzle.forbiddenSpots,
			nullptr
		);
		assert(!newMoves.empty() && "At least some moves should be possible");
		assert(newMoves.size() == (runnerMoves + blockMoves) && "We should have 6 moves discovered");
	}
	
	{
		const auto blockMoves = 4;
		auto newMoves = moveDiscovery.gatherMoves(
			largePuzzle.dimensions,
			largePuzzle.boardState,
			largePuzzle.forbiddenSpots,
			nullptr
		);
		assert(!newMoves.empty() && "At least some moves should be possible");
		//std::cout << "discover " << newMoves.size() << " moves" << std::endl;
		assert(newMoves.size() == blockMoves && "We should have 4 moves discovered");
	}
}

void testMoving() {
	using BoardType = std::remove_const<decltype(largePuzzle.boardState)>::type;

	Move moveRight{ largePuzzle.boardState, largePuzzle.boardState->blocks[0], 1, Direction::Right };
	assert(largePuzzle.boardState.get() == moveRight.boardState.get() && "A move should not copy the state, but only reference the original state");

	const auto afterMoveRight = moveRight.proceed();
	assert(largePuzzle.boardState->blocks[0].move(Direction::Right) == afterMoveRight->blocks[0]);
	assert(largePuzzle.boardState->blocks[0].move(Direction::Left)  != afterMoveRight->blocks[0]);
	assert(largePuzzle.boardState->blocks[0].move(Direction::Up)    != afterMoveRight->blocks[0]);
	assert(largePuzzle.boardState->blocks[0].move(Direction::Down)  != afterMoveRight->blocks[0]);
	assert(largePuzzle.boardState->blocks[0].id == afterMoveRight->blocks[0].id);

	assert(largePuzzle.boardState->moveNumber + 1 == afterMoveRight->moveNumber);

	{
		Block middle({ 2, 2 }, { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } }, "blockMid");
		assert(middle == (Block({ 0, 0 }, { { 2, 2 }, { 3, 2 }, { 2, 3 }, { 3, 3 } }, "blockMid")));
		assert(middle == (Block({ 0, 0 }, { { 3, 3 }, { 2, 2 }, { 3, 2 }, { 2, 3 } }, "blockMid")));

		assert(middle.move(Direction::Up)    == (Block({ 2, 1 }, { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } }, "blockMid")));
		assert(middle.move(Direction::Down)  == (Block({ 2, 3 }, { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } }, "blockMid")));
		assert(middle.move(Direction::Left)  == (Block({ 1, 2 }, { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } }, "blockMid")));
		assert(middle.move(Direction::Right) == (Block({ 3, 2 }, { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } }, "blockMid")));
	}

}

void testHashing() {
	// Test if type can be constructed
	BoardHasher<int> emptyHasher{ emptyPuzzle };
	BoardHasher<int> initialHasher{ largePuzzle };

	assert(emptyHasher.hash(*(emptyPuzzle.boardState))   == emptyHasher.hash(*(emptyPuzzle.boardState)) && "Hash for identical states should be equal");
	assert(initialHasher.hash(*(largePuzzle.boardState)) == initialHasher.hash(*(largePuzzle.boardState)) && "Hash for identical states should be equal");
	assert(initialHasher.hash(*(largePuzzle.boardState)) != emptyHasher.hash(*(emptyPuzzle.boardState)) && "Hash for different states should be different");

	auto swappedBlocks = largePuzzle.boardState->blocks;
	auto temp= swappedBlocks[0];
	swappedBlocks[0].id = swappedBlocks[1].id;
	swappedBlocks[1].id = temp.id;
	BoardState swappedState{
		0,
		largePuzzle.boardState->runner,
		swappedBlocks 
	};
	assert(initialHasher.hash(swappedState) == initialHasher.hash(*(largePuzzle.boardState)) && "Hash should be the same for 2 same-sized blocks in swapped positions");
}

auto testSolverFx = [&](Solver solver, const bool& benchmark, std::string expectedOut = "") {
	std::stringstream out;
	std::list<Puzzle> result;
	if (benchmark) {
		result = solver.solve(&out);
	} else {
		result = solver.solve();
	}

	if ("" != expectedOut) {
		printText(result, out);
		assert(compareString(out.str(), expectedOut));
	}

	std::cout << "found solution in " << (result.size() - 1) << " steps" << std::endl;
	return result;
};

void testSolver() {
	MoveRunnerFirst moveDiscovery;
	assert( 4 == testSolverFx(Solver(tinyPuzzle, moveDiscovery), true).size());

	assert( 4 == testSolverFx(Solver(tinyPuzzle, moveDiscovery), false,
		"\n"
		"-------------- step:0 move:0\n"
		"3 3\n"
		"1 1 @\n"
		"#####\n"
		"#@  #\n"
		"# A #\n"
		"#   #\n"
		"#####\n"
		"\n"
		"-------------- step:1 move:1\n"
		"3 3\n"
		"1 1 @\n"
		"#####\n"
		"#   #\n"
		"#@A #\n"
		"#   #\n"
		"#####\n"
		"\n"
		"-------------- step:2 move:2\n"
		"3 3\n"
		"1 1 @\n"
		"#####\n"
		"#   #\n"
		"#@^ #\n"
		"# A #\n"
		"#####\n"
		"\n"
		"-------------- step:3 move:3\n"
		"3 3\n"
		"1 1 @\n"
		"#####\n"
		"#   #\n"
		"# @ #\n"
		"# A #\n"
		"#####\n"
	).size());
	assert( 5 == testSolverFx(Solver(emptyPuzzle,  moveDiscovery), false).size());
	assert( 9 == testSolverFx(Solver(smallPuzzle,  moveDiscovery), false).size());
	assert( 9 == testSolverFx(Solver(tetrisPuzzle, moveDiscovery), false).size());
	assert(39 == testSolverFx(Solver(largePuzzle,  moveDiscovery), false).size());
}

int main(int /*argc*/, char* /*argv*/[]) {
	testPoint();
	testBlocks();
	testPuzzles();
	testPrintScan();
	testMoveValidation();
	testMoveDiscovery();
	testMoving();
	testHashing();
	testSolver();
}

