// snake.h - A snake's body, current/pending heading and grid movement. No Qt dependency.
#ifndef SNAKE_SNAKE_H
#define SNAKE_SNAKE_H
#include "types.h"
#include <cstddef>
#include <deque>

class Snake {
public:
    void reset(const std::deque<Cell> &body, Direction direction);
    bool requestTurn(Direction newDirection);
    Cell proposedHead() const;
    void advance(Cell newHead, bool grow);
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
    std::deque<Cell> body_;
    Direction direction_ = Direction::Right;
    Direction pendingDirection_ = Direction::Right;
};
#endif
