#include "solver.h"

#include "printer.h"
#include <chrono>

bool isSolution(const BoardState& boardState, const Point& goal) {
	return (boardState.runner.shift.x == goal.x) && (boardState.runner.shift.y == goal.y);
}

Solver::Solver(const Puzzle& puzzle, const MoveDiscovery& moveDiscovery)
	: puzzle{ puzzle }
	, hasher{ puzzle }
	, moveDiscovery{ moveDiscovery }
{
	const auto initialMoves = moveDiscovery.gatherMoves(
		puzzle.dimensions,
		puzzle.boardState,
		puzzle.forbiddenSpots
	);
	for (auto& move : initialMoves) {
		possibleMoveStack.push_back(std::move(move));
	}
}

std::list<Puzzle> Solver::solve(std::ostream* debugOut) {
	return (nullptr == debugOut) ? solveFast() : solveDebug(debugOut);
}

std::list<Puzzle> Solver::solveDebug(std::ostream* debugOut) {
	printText(puzzle, *debugOut);

	auto globalTime = std::chrono::high_resolution_clock::now();
	auto pickMoveTime = std::chrono::high_resolution_clock::duration{};
	auto solutionCheckTime = std::chrono::high_resolution_clock::duration{};
	auto hashTime = std::chrono::high_resolution_clock::duration{};
	auto lookupTime = std::chrono::high_resolution_clock::duration{};
	auto gatherMovesTime = std::chrono::high_resolution_clock::duration{};
	auto insertMovesTime = std::chrono::high_resolution_clock::duration{};

	while (!possibleMoveStack.empty()) {
		auto start_time = std::chrono::high_resolution_clock::now();

		Move firstMove = *(begin(possibleMoveStack));
		// Note: invalidates firstMoveIt
		possibleMoveStack.pop_front();

		std::shared_ptr<BoardState> boardStateAfterMove = firstMove.proceed();

		auto end_time = std::chrono::high_resolution_clock::now();
		pickMoveTime += (end_time - start_time);
		start_time = std::chrono::high_resolution_clock::now();

		if (isSolution(*boardStateAfterMove, puzzle.goal)) {
			*debugOut << "Solution is:" << std::endl;
			printText(Puzzle{ puzzle.dimensions, puzzle.goal, puzzle.forbiddenSpots, boardStateAfterMove }, *debugOut);
			*debugOut << "numberOfMoves: " << boardStateAfterMove->numberOfMoves << std::endl;
			const auto hash = hasher.hash(*boardStateAfterMove);
			*debugOut << "Final hash: " << hash << std::endl;

			auto result = solution(firstMove.boardState, boardStateAfterMove);

			*debugOut << hasher.hash(*(puzzle.boardState)) << std::endl;
			printText(puzzle, *debugOut);
			*debugOut << std::endl << "-------------- " << std::endl;

			const auto now = std::chrono::high_resolution_clock::now();
			const auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - globalTime).count();
			*debugOut << "Total time was:    " << totalTime << std::endl;
			if (0 < totalTime) {
				*debugOut
					<< "pickMoveTime:      " << (100 * std::chrono::duration_cast<std::chrono::milliseconds>(pickMoveTime).count()      / totalTime) << "%" << std::endl
					<< "solutionCheckTime: " << (100 * std::chrono::duration_cast<std::chrono::milliseconds>(solutionCheckTime).count() / totalTime) << "%" << std::endl
					<< "hashTime:          " << (100 * std::chrono::duration_cast<std::chrono::milliseconds>(hashTime).count()          / totalTime) << "%" << std::endl
					<< "lookupTime:        " << (100 * std::chrono::duration_cast<std::chrono::milliseconds>(lookupTime).count()        / totalTime) << "%" << std::endl
					<< "gatherMovesTime:   " << (100 * std::chrono::duration_cast<std::chrono::milliseconds>(gatherMovesTime).count()   / totalTime) << "%" << std::endl
					<< "insertMovesTime:   " << (100 * std::chrono::duration_cast<std::chrono::milliseconds>(insertMovesTime).count()   / totalTime) << "%" << std::endl
				;
			}
			return result;
		}

		end_time = std::chrono::high_resolution_clock::now();
		solutionCheckTime += (end_time - start_time);
		start_time = std::chrono::high_resolution_clock::now();

		const auto boardStateAfterMoveHash = hasher.hash(*boardStateAfterMove);

		end_time = std::chrono::high_resolution_clock::now();
		hashTime += (end_time - start_time);
		start_time = std::chrono::high_resolution_clock::now();

		auto& numberOfMoves = knownPaths[boardStateAfterMoveHash];
		
		end_time = std::chrono::high_resolution_clock::now();
		lookupTime += (end_time - start_time);

		if (numberOfMoves == 0) {
			numberOfMoves = boardStateAfterMove->numberOfMoves;

			parentsOf[boardStateAfterMoveHash].push_back({ hasher.hash(*firstMove.boardState), boardStateAfterMove });

			start_time = std::chrono::high_resolution_clock::now();

			// Queue follow-up moves
			const auto newMoves = moveDiscovery.gatherMoves(
				puzzle.dimensions,
				std::move(boardStateAfterMove),
				puzzle.forbiddenSpots
			);

			end_time = std::chrono::high_resolution_clock::now();
			gatherMovesTime += (end_time - start_time);
			start_time = std::chrono::high_resolution_clock::now();

			for (auto& move : newMoves) {
				possibleMoveStack.push_back(std::move(move));
			}

			end_time = std::chrono::high_resolution_clock::now();
			insertMovesTime += (end_time - start_time);
		}
	}

	return {};
}

std::list<Puzzle> Solver::solveFast() {
	// Note: This can be distributed over multiple threads
	// Just lock the datastructure possibleMoveStack when popping moves & reinserting moves
	// It's not necessary to check the latest state of possibleMoveStack, worst case scenario
	// a few already-known moves will be queued twice
	while (!possibleMoveStack.empty()) {
		//peek a move
		Move firstMove = *(begin(possibleMoveStack));
		// Note: invalidates firstMoveIt
		possibleMoveStack.pop_front();

		std::shared_ptr<BoardState> boardStateAfterMove(firstMove.proceed());

		if (isSolution(*boardStateAfterMove, puzzle.goal)) {
			return solution(firstMove.boardState, boardStateAfterMove);
		}

		const auto boardStateAfterMoveHash = hasher.hash(*boardStateAfterMove);
		auto& numberOfMoves = knownPaths[boardStateAfterMoveHash];
		if (numberOfMoves == 0) {
			//unknown paths to add
			numberOfMoves = boardStateAfterMove->numberOfMoves;

			parentsOf[boardStateAfterMoveHash].push_back({ hasher.hash(*firstMove.boardState), boardStateAfterMove });

			// Queue follow-up moves
			const auto newMoves = moveDiscovery.gatherMoves(
				puzzle.dimensions,
				std::move(boardStateAfterMove),
				puzzle.forbiddenSpots
			);

			for (auto& move : newMoves) {
				possibleMoveStack.push_back(std::move(move));
			}
		}
	}

	return {};
}

std::list<Puzzle> Solver::solution(std::shared_ptr<BoardState> firstBoardState, std::shared_ptr<BoardState> lastBoardState) {
	std::list<Puzzle> result;
	result.push_front({ puzzle.dimensions, puzzle.goal, puzzle.forbiddenSpots, lastBoardState });

	const auto initialHash = hasher.hash(*(puzzle.boardState));
	HashType stopHash = hasher.hash(*firstBoardState);
	while (stopHash != 0 && stopHash != initialHash) {
		auto start = parentsOf[stopHash];
		if (start.size() > 0 && start[0].first != 0) {
			result.push_front({ puzzle.dimensions, puzzle.goal, puzzle.forbiddenSpots, start[0].second });
			stopHash = start[0].first;
		}
	}
	result.push_front(puzzle);
	return result;
}

