#pragma once

#include <memory>
#include <vector>

#include "block.h"

struct BoardState
{
    // Numer of moves made since the start position
    const int m_numberOfMovesFromStart;

    // Puzzle piece to be moved to the goal
    const Block m_runner;

    // all movable bystander blocks on the board
    const std::vector<Block> m_blocks;
};

struct Move
{
    // State from which to move
    std::shared_ptr<BoardState> m_state;

    // Block to be moved
    const Block &m_block;

    // Direction in which to move block
    Direction m_directionToMove;

    // Returns the BoardState after the move
    BoardState operator()();

    Move(std::shared_ptr<BoardState> state, const Block &block, Direction dir)
        : m_state{ state }
        , m_block{ block }
        , m_directionToMove{ dir }
    {
    }
};

struct Puzzle
{
    // Dimensions of the puzzle:
    // X: [ 0, width - 1]
    // Y: [ 0, height - 1]
    const Point m_dimensions;

    // Position of the runner required in order to finish the puzzle
    const Block m_goal;

    // Positions (in addition to regular border) where blocks cannot be placed
    const std::vector<Point> m_forbiddenSpots;

    // Initial position of all blocks on the board
    const BoardState m_initialState;
};

bool valid(const Puzzle &puzzle);
