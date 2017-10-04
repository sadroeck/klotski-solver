#pragma once

#include "puzzle.h"

struct DefaultMoveValidation
{
    // Checks if the proposed position of the block against the current state of the board
    // is a valid one.
    // The position of the current block is ignored
    static bool validBlockPosition(
        const Block &block,
        const Point &dims,
        const BoardState &boardState,
        const std::vector<Point> invalidPositions);
};