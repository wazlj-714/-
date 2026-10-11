// snake.h：封装蛇体、移动方向和候选方向。
#ifndef SNAKE_SNAKE_H
#define SNAKE_SNAKE_H
#include "types.h"
#include <cstddef>
#include <deque>

// 蛇头位于队列首端，蛇尾位于末端；外部只能通过只读接口查看蛇体。
class Snake {
public:
    void reset(const std::deque<Cell> &body, Direction direction);
    // 申请候选方向，实际移动时才提交。
    bool requestTurn(Direction newDirection);
    // 预测下一蛇头，供 Game 在移动前判断碰撞。
    Cell proposedHead() const;
    void advance(Cell newHead, bool grow);
    // 只检查前 count 节，便于排除本回合将释放的尾格。
    bool contains(Cell point, std::size_t count) const;
    bool contains(Cell point) const { return contains(point, body_.size()); }
    bool empty() const { return body_.empty(); }
    std::size_t size() const { return body_.size(); }
    const std::deque<Cell> &body() const { return body_; }
    Direction direction() const { return direction_; }
    Direction pendingDirection() const { return pendingDirection_; }
    void commitDirection() { direction_ = pendingDirection_; }
    void clear() { body_.clear(); }
private:
    // 方向分别表示当前实际朝向和下一回合候选朝向。
    std::deque<Cell> body_;
    Direction direction_ = Direction::Right;
    Direction pendingDirection_ = Direction::Right;
};
#endif
