// snake.cpp - deque enables constant-time addition at the head and removal at the tail.
#include "snake.h"
void Snake::reset(const std::deque<Cell> &body, Direction direction) {
    body_ = body;
    direction_ = pendingDirection_ = direction;
}
bool Snake::requestTurn(Direction newDirection) {
    // Compare with the committed direction: two rapid key presses cannot reverse in one tick.
    if (body_.size() > 1 && opposite(newDirection, direction_)) return false;
    pendingDirection_ = newDirection;
    return true;
}
Cell Snake::proposedHead() const {
    return body_.empty() ? Cell{} : nextCell(body_.front(), pendingDirection_);
}
void Snake::advance(Cell newHead, bool grow) {
    commitDirection();
    body_.push_front(newHead);
    if (!grow) body_.pop_back();
}
bool Snake::contains(Cell point, std::size_t count) const {
    if (count > body_.size()) count = body_.size();
    for (std::size_t i = 0; i < count; ++i) {
        if (body_[i] == point) return true;
    }
    return false;
}
