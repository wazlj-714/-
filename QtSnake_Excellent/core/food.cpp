// food.cpp：枚举所有空格后等概率选择，边界格也可以生成食物。
#include "food.h"
#include <vector>
bool Food::generate(int width, int height, const Snake &a, const Snake &b,
                    std::mt19937 &rng) {
    // 先标记双方蛇体占用的格点，避免对每个格点重复扫描蛇体。
    std::vector<unsigned char> occupied(static_cast<std::size_t>(width * height), 0);
    const auto markSnake = [&](const Snake &snake) {
        for (const auto &cell : snake.body())
            if (cell.x >= 0 && cell.x < width && cell.y >= 0 && cell.y < height)
                occupied[static_cast<std::size_t>(cell.y * width + cell.x)] = 1;
    };
    markSnake(a);
    markSnake(b);
    std::vector<Cell> freeCells;
    freeCells.reserve(occupied.size());
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            if (!occupied[static_cast<std::size_t>(y * width + x)])
                freeCells.push_back({x,y});
    // 满盘时立即返回，避免重复随机查找导致无法结束。
    if (freeCells.empty()) { clear(); return false; }
    // 对空格列表的下标均匀采样，保证每个空格被选中的概率相同。
    std::uniform_int_distribution<std::size_t> distribution(0, freeCells.size() - 1);
    restore(freeCells[distribution(rng)]);
    return true;
}
