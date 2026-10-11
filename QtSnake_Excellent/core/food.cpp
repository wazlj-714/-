// food.cpp - Uniform O(board cells + snake length) sampling, including boundary cells.
#include "food.h"
#include <vector>
bool Food::generate(int width, int height, const Snake &a, const Snake &b,
                    std::mt19937 &rng) {
    // Build an occupancy bitmap once instead of checking every cell against both deques.
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
    if (freeCells.empty()) { clear(); return false; }
    std::uniform_int_distribution<std::size_t> distribution(0, freeCells.size() - 1);
    restore(freeCells[distribution(rng)]);
    return true;
}
