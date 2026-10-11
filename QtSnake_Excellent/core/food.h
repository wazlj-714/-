// food.h：管理食物位置、存在标记和分值，不负责绘制。
#ifndef SNAKE_FOOD_H
#define SNAKE_FOOD_H
#include "snake.h"
#include <random>
class Food {
public:
    static constexpr int NORMAL_POINTS = 10;
    // 从双方蛇体以外的空格中生成食物；没有空格时返回失败。
    bool generate(int width, int height, const Snake &a, const Snake &b, std::mt19937 &rng);
    Cell position() const { return position_; }
    int points() const { return NORMAL_POINTS; }
    bool present() const { return present_; }
    // 恢复已经由 Game 校验过的位置，并将食物标记为存在。
    void restore(Cell position) { position_ = position; present_ = true; }
    void clear() { present_ = false; }
private:
    Cell position_{};
    bool present_ = false;
};
#endif
