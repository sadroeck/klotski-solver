#pragma once

#include <vector>
#include <memory>

#include "block.h"
#include "puzzle.h"
#include "MoveValidation.h"

template <typename Validation = DefaultMoveValidation>
class MoveRunnerFirst
{
public:
    static std::vector<Move> gatherMoves(
        const Point dimensions,
        const std::shared_ptr<BoardState> &currentState,
        const std::vector<Point> &invalidPositions,
        Direction previousDirection = Direction::Number_of_dirs);
};