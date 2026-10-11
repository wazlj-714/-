// food.h - Food spawn and score value; neither painting nor Qt types are used.
#ifndef SNAKE_FOOD_H
#define SNAKE_FOOD_H
#include "snake.h"
#include <random>
class Food {
public:
    static constexpr int NORMAL_POINTS = 10;
    bool generate(int width, int height, const Snake &a, const Snake &b, std::mt19937 &rng);
    Cell position() const { return position_; }
    int points() const { return NORMAL_POINTS; }
    bool present() const { return present_; }
    void restore(Cell position) { position_ = position; present_ = true; }
    void clear() { present_ = false; }
private:
    Cell position_{};
    bool present_ = false;
};
#endif
