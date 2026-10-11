// game.cpp - Two-player moves are resolved simultaneously, including vacated tails.
#include "game.h"
#include <algorithm>
#include <fstream>
#include <set>
#include <utility>
#include <vector>

namespace {
bool validDirection(int x) { return x >= 0 && x <= 3; }
bool validDifficulty(int x) { return x >= 0 && x <= 2; }
bool validMode(int x) { return x == 1 || x == 2; }
}
Game::Game(int width, int height, std::uint32_t seed)
    : width_(std::max(8, std::min(100, width))),
      height_(std::max(8, std::min(100, height))),
      rng_(seed ? seed : std::random_device{}()) {}

void Game::transition(GameState &next) {
    if (active_ == &next) return;
    active_->onExit(*this);
    active_ = &next;
    active_->onEnter(*this);
    notifyChanged();
}
void Game::resetSnakes() {
    std::deque<Cell> first;
    // Keep initial snakes inside even a minimal 8-column board.
    const int initialLength = std::min(4, std::max(2, width_ / 4));
    const int y1 = height_ * 3 / 4;
    const int head1x = width_ * 3 / 4 - 1;
    for (int i = 0; i < initialLength; ++i) first.push_back({head1x + i, y1});
    snake1_.reset(first, Direction::Left);
    snake2_.clear();
    if (mode_ == GameMode::Dual) {
        std::deque<Cell> second;
        const int y2 = height_ / 4;
        const int head2x = width_ / 4;
        for (int i = 0; i < initialLength; ++i) second.push_back({head2x - i, y2});
        snake2_.reset(second, Direction::Right);
    }
}
void Game::start(GameMode mode, Difficulty difficulty) {
    mode_ = mode;
    difficulty_ = difficulty;
    score1_ = score2_ = frames_ = 0;
    winner_ = Winner::None;
    endReason_ = EndReason::None;
    paused_ = false;
    resetSnakes();
    if (!food_.generate(width_, height_, snake1_, snake2_, rng_)) {
        finish(EndReason::BoardFull);
        return;
    }
    transition(playState_);
    notifyChanged();
}
void Game::backToMenu() {
    paused_ = false;
    transition(menuState_);
}
void Game::pause() {
    if (scene() == Scene::Playing) { paused_ = true; notifyChanged(); }
}
void Game::resume() {
    if (scene() == Scene::Playing) { paused_ = false; notifyChanged(); }
}
bool Game::turn(int player, Direction direction) {
    if (scene() != Scene::Playing || paused_) return false;
    if (player == 1) return snake1_.requestTurn(direction);
    if (player == 2 && mode_ == GameMode::Dual) return snake2_.requestTurn(direction);
    return false;
}
void Game::tick() {
    if (scene() == Scene::Playing && !paused_) active_->update(*this);
}
bool Game::outOfBounds(Cell c) const {
    return c.x < 0 || c.y < 0 || c.x >= width_ || c.y >= height_;
}
void Game::advanceFrame() {
    // Snapshot both proposed positions before modifying either deque.
    const bool dual = mode_ == GameMode::Dual;
    const Cell next1 = snake1_.proposedHead();
    const Cell next2 = dual ? snake2_.proposedHead() : Cell{};
    const bool eat1 = food_.present() && next1 == food_.position();
    const bool eat2 = dual && food_.present() && next2 == food_.position();
    bool dead1 = outOfBounds(next1);
    bool dead2 = dual && outOfBounds(next2);
    bool hitBody = false;

    // The tail vacates on a non-growth move; checking against it is a false collision.
    if (!dead1 && snake1_.contains(next1, snake1_.size() - (eat1 ? 0 : 1))) {
        dead1 = true;
        hitBody = true;
    }
    if (dual) {
        if (!dead2 && snake2_.contains(next2, snake2_.size() - (eat2 ? 0 : 1))) {
            dead2 = true;
            hitBody = true;
        }
        if (!dead1 && snake2_.contains(next1, snake2_.size() - (eat2 ? 0 : 1))) {
            dead1 = true;
            hitBody = true;
        }
        if (!dead2 && snake1_.contains(next2, snake1_.size() - (eat1 ? 0 : 1))) {
            dead2 = true;
            hitBody = true;
        }
        if (next1 == next2) {
            dead1 = dead2 = true;
            finish(EndReason::HeadOn, true, true);
            return;
        }
    }
    if (dead1 || dead2) {
        finish(hitBody ? EndReason::Body : EndReason::Wall, dead1, dead2);
        return;
    }
    snake1_.advance(next1, eat1);
    if (dual) snake2_.advance(next2, eat2);
    if (eat1) score1_ += food_.points();
    if (eat2) score2_ += food_.points();
    ++frames_;
    if ((eat1 || eat2) && !food_.generate(width_, height_, snake1_, snake2_, rng_))
        finish(EndReason::BoardFull);
    notifyChanged();
}
void Game::finish(EndReason reason, bool dead1, bool dead2) {
    if (scene() == Scene::GameOver) return;
    endReason_ = reason;
    if (mode_ == GameMode::Dual) {
        if (reason == EndReason::BoardFull) {
            winner_ = score1_ > score2_ ? Winner::Player1 :
                      score2_ > score1_ ? Winner::Player2 : Winner::Draw;
        } else {
            winner_ = dead1 && dead2 ? Winner::Draw :
                      dead1 ? Winner::Player2 : Winner::Player1;
        }
    } else {
        winner_ = reason == EndReason::BoardFull ? Winner::Player1 : Winner::None;
    }
    if (score1_ > 0) leaderboard_.add({score1_, "P1", mode_, difficulty_});
    if (mode_ == GameMode::Dual && score2_ > 0)
        leaderboard_.add({score2_, "P2", mode_, difficulty_});
    if (!leaderboardPath_.empty()) leaderboard_.save(leaderboardPath_);
    paused_ = true;
    transition(gameOverState_);
}
int Game::stepIntervalMs() const {
    const int base = difficulty_ == Difficulty::Easy ? 260 :
                     difficulty_ == Difficulty::Normal ? 170 : 100;
    const int minimum = difficulty_ == Difficulty::Easy ? 130 :
                        difficulty_ == Difficulty::Normal ? 80 : 55;
    const int bestScore = std::max(score1_, score2_);
    return std::max(minimum, base - (bestScore / 50) * 9);
}
bool Game::loadLeaderboard(const std::string &path, std::string *error) {
    // Set the persistence location only if the data was successfully parsed.
    leaderboardPath_ = path;
    return leaderboard_.load(path, error);
}
bool Game::saveLeaderboard(std::string *error) const {
    if (leaderboardPath_.empty()) {
        if (error) *error = "Leaderboard path has not been configured";
        return false;
    }
    return leaderboard_.save(leaderboardPath_, error);
}
bool Game::saveSnapshot(const std::string &path, std::string *error) const {
    if (scene() != Scene::Playing) {
        if (error) *error = "Only an ongoing game can be saved";
        return false;
    }
    std::ofstream out(path, std::ios::trunc);
    if (!out) { if (error) *error = "Cannot open save file"; return false; }
    out << "SNAKE_SAVE_V1\n" << width_ << ' ' << height_ << '\n';
    out << static_cast<int>(mode_) << ' ' << static_cast<int>(difficulty_) << ' '
        << score1_ << ' ' << score2_ << ' ' << frames_ << '\n';
    auto writeSnake = [&out](const Snake &snake) {
        out << static_cast<int>(snake.direction()) << ' '
            << static_cast<int>(snake.pendingDirection()) << ' ' << snake.size() << '\n';
        for (const auto &c : snake.body()) out << c.x << ' ' << c.y << '\n';
    };
    writeSnake(snake1_);
    writeSnake(snake2_);
    out << food_.position().x << ' ' << food_.position().y << '\n';
    out.flush();
    if (!out) { if (error) *error = "Save file write failed"; return false; }
    return true;
}
bool Game::loadSnapshot(const std::string &path, std::string *error) {
    std::ifstream in(path);
    if (!in) { if (error) *error = "Save file not found"; return false; }
    std::string magic;
    int width = 0, height = 0, mode = 0, difficulty = 0, s1 = 0, s2 = 0, frames = 0;
    if (!(in >> magic >> width >> height >> mode >> difficulty >> s1 >> s2 >> frames) ||
        magic != "SNAKE_SAVE_V1" || width != width_ || height != height_ ||
        !validMode(mode) || !validDifficulty(difficulty) ||
        s1 < 0 || s1 > 1000000 || s2 < 0 || s2 > 1000000 ||
        frames < 0 || frames > 10000000) {
        if (error) *error = "Invalid save header or board size";
        return false;
    }
    struct LoadedSnake { std::deque<Cell> body; Direction dir; Direction queued; };
    auto readSnake = [&](LoadedSnake &snake) -> bool {
        int direction = -1, pending = -1, count = -1;
        if (!(in >> direction >> pending >> count) || !validDirection(direction) ||
            !validDirection(pending) || count < 0 || count > width_ * height_) return false;
        snake.dir = static_cast<Direction>(direction);
        snake.queued = static_cast<Direction>(pending);
        if (count > 1 && opposite(snake.dir, snake.queued)) return false;
        for (int i = 0; i < count; ++i) {
            Cell c;
            if (!(in >> c.x >> c.y) || outOfBounds(c)) return false;
            if (!snake.body.empty() && manhattan(snake.body.back(), c) != 1) return false;
            snake.body.push_back(c);
        }
        return true;
    };
    LoadedSnake l1, l2;
    Cell food;
    if (!readSnake(l1) || !readSnake(l2) || !(in >> food.x >> food.y) ||
        outOfBounds(food) || l1.body.empty() ||
        ((mode == 1) && !l2.body.empty()) || ((mode == 2) && l2.body.empty()) ||
        ((mode == 1) && s2 != 0)) {
        if (error) *error = "Invalid snake or food data";
        return false;
    }
    std::string extra;
    if (in >> extra) {
        if (error) *error = "Unexpected save data";
        return false;
    }
    std::set<std::pair<int, int>> occupied;
    auto checkUnique = [&occupied](const std::deque<Cell> &body) {
        for (auto c : body)
            if (!occupied.insert({c.x, c.y}).second) return false;
        return true;
    };
    if (!checkUnique(l1.body) || !checkUnique(l2.body) ||
        occupied.count({food.x, food.y}) != 0) {
        if (error) *error = "Overlapping snake segments or food";
        return false;
    }
    // Commit only after all fields pass validation: a corrupt save cannot damage the game.
    Snake newFirst, newSecond;
    newFirst.reset(l1.body, l1.dir);
    newSecond.reset(l2.body, l2.dir);
    newFirst.requestTurn(l1.queued);
    if (!newSecond.empty()) newSecond.requestTurn(l2.queued);
    snake1_ = std::move(newFirst);
    snake2_ = std::move(newSecond);
    food_.restore(food);
    score1_ = s1;
    score2_ = s2;
    frames_ = frames;
    mode_ = static_cast<GameMode>(mode);
    difficulty_ = static_cast<Difficulty>(difficulty);
    winner_ = Winner::None;
    endReason_ = EndReason::None;
    paused_ = true; // Load safely. The player explicitly presses 'Continue'.
    transition(playState_);
    notifyChanged();
    return true;
}
