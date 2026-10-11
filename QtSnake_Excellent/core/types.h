// types.h - Plain C++ data types shared by the snake game business layer.
#ifndef SNAKE_TYPES_H
#define SNAKE_TYPES_H

#include <cstdlib>

struct Cell {
    int x = 0;
    int y = 0;
};
inline bool operator==(const Cell &a, const Cell &b) { return a.x == b.x && a.y == b.y; }
inline bool operator!=(const Cell &a, const Cell &b) { return !(a == b); }
inline int manhattan(const Cell &a, const Cell &b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

enum class Direction { Up = 0, Down = 1, Left = 2, Right = 3 };
enum class GameMode { Single = 1, Dual = 2 };
enum class Difficulty { Easy = 0, Normal = 1, Hard = 2 };
enum class Scene { Menu, Playing, GameOver };
enum class EndReason { None, Wall, Body, HeadOn, BoardFull };
enum class Winner { None, Player1, Player2, Draw };

inline Cell nextCell(Cell head, Direction direction) {
    switch (direction) {
    case Direction::Up:    --head.y; break;
    case Direction::Down:  ++head.y; break;
    case Direction::Left:  --head.x; break;
    case Direction::Right: ++head.x; break;
    }
    return head;
}
inline bool opposite(Direction a, Direction b) {
    return (a == Direction::Up && b == Direction::Down) ||
           (a == Direction::Down && b == Direction::Up) ||
           (a == Direction::Left && b == Direction::Right) ||
           (a == Direction::Right && b == Direction::Left);
}
#endif
