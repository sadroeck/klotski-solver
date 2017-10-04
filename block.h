#pragma once

#include <string>
#include <vector>

enum Direction
{
    Down,
    Right,
    Left,
    Up,
    Number_of_dirs
};

const struct Block
{
    int m_startX;
    int m_startY;
    int m_sizeX;
    int m_sizeY;
    std::string id;
};

const struct Point
{
    int m_x;
    int m_y;
};

bool nextToFreeSpace(const Block &block, const std::vector<Point> &freeSpaces);
bool overlaps(const Block &left, const Block &right);
Block move(const Block &block, Direction dir);
bool same(const Block &left, const Block &right);
bool equalPosition(const Block &left, const Block &right);