#pragma once
#include "puzzle.h"

#include <random>
#include <algorithm> // std::find

// Need a hash that's both order invariant & block-id invariant
// i.e. 2 blocks with different ids at the same position should still hash the same
// i.e. hash should be the same regardless of the order in which the blocks on the board are hashed

// TODO : As we only update 1 block at a time, this would be perfect to update incrementally

template <typename HashType/* = int*/>
class BoardHasher {
public:
	HashType hash(const BoardState& boardState) {
		HashType result{};
		result ^= runnerHash(boardState.runner);
		for (const auto& block : boardState.blocks) {
			result ^= blockHash(block);
		}
		return result;
	};

	BoardHasher(const Puzzle& puzzle)
		: dimensions{ puzzle.dimensions }
		, numberOfPuzzlePos{ puzzle.dimensions.x * puzzle.dimensions.y }
		, hashList{}
		, blockPointSetSet{}
	{
		for (const auto block : puzzle.boardState->blocks) {
			blockPointSetSet.insert(block.pointSet);
		}

		std::mt19937 generator{ (std::random_device{})() };
		std::uniform_int_distribution<HashType> distribution{};

		const auto numberOfBlockSetPos = (blockPointSetSet.size() + 1) * numberOfPuzzlePos;
		for (auto blockSetPos = 0u; blockSetPos < numberOfBlockSetPos; ++blockSetPos) {
			hashList.push_back(distribution(generator));
		}
	}

private:
	int blockHash(const Block& block) {
		const auto blockPos = std::distance(
			begin(blockPointSetSet),
			std::find_if(
				begin(blockPointSetSet),
				end(blockPointSetSet),
				[&](const auto& b) { return b == block.pointSet; }
			)
		);

		// First hash is runner, so index offset of the other blocks is 1
		return hashList[((blockPos + 1) * numberOfPuzzlePos) + (block.shift.x * dimensions.y) + block.shift.y];
	}

	int runnerHash(const Block& runner) {
		// First blockType is the runner : blockPos == 0
		return hashList[(runner.shift.x * dimensions.y) + runner.shift.y];
	}

	// Dimensions of the playing field
	const Point dimensions;
	const int numberOfPuzzlePos;

	// unique hash for each blocktype positioned on the board
	// Note: indexing scheme = [blockType][x][y]
	std::vector<HashType> hashList;

	// Unique hash for each block size
	std::set<std::set<Point>> blockPointSetSet;
};

