// types.h：业务层共用的数据记录与枚举，不依赖 Qt。
#ifndef SNAKE_TYPES_H
#define SNAKE_TYPES_H

#include <cstdlib>

// 格点记录：只表达棋盘坐标，不包含界面像素或颜色。
struct Cell {
    int x = 0;
    int y = 0;
};
inline bool operator==(const Cell &a, const Cell &b) { return a.x == b.x && a.y == b.y; }
inline bool operator!=(const Cell &a, const Cell &b) { return !(a == b); }
// 相邻蛇节的距离应为一个格点，读档时据此检查连续性。
inline int manhattan(const Cell &a, const Cell &b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

// 枚举数值也用于文件格式，调整取值时需同步修改存档校验。
enum class Direction { Up = 0, Down = 1, Left = 2, Right = 3 };
enum class GameMode { Single = 1, Dual = 2 };
enum class Difficulty { Easy = 0, Normal = 1, Hard = 2 };
enum class Scene { Menu, Playing, GameOver };
enum class EndReason { None, Wall, Body, HeadOn, BoardFull };
enum class Winner { None, Player1, Player2, Draw };

// 根据方向计算下一格，不在此处判断碰撞。
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
