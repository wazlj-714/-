// snake.cpp：使用双端队列完成蛇头插入和蛇尾删除。
#include "snake.h"
// 开局或读档时同时初始化蛇体与两个方向。
void Snake::reset(const std::deque<Cell> &body, Direction direction) {
    body_ = body;
    direction_ = pendingDirection_ = direction;
}
bool Snake::requestTurn(Direction newDirection) {
    // 与已提交方向比较，防止同一回合内连续按键绕过反向限制。
    if (body_.size() > 1 && opposite(newDirection, direction_)) return false;
    pendingDirection_ = newDirection;
    return true;
}
Cell Snake::proposedHead() const {
    return body_.empty() ? Cell{} : nextCell(body_.front(), pendingDirection_);
}
// 加入新头；普通移动删除尾部，进食时保留尾部以增长。
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
