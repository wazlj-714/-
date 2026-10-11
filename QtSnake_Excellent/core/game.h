// game.h - Qt-free game orchestrator: state transitions, collisions, persistence.
#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H
#include "food.h"
#include "gamestate.h"
#include "leaderboard.h"
#include <cstdint>
#include <functional>
#include <random>
#include <string>
#include <utility>

class Game {
public:
    explicit Game(int width = 20, int height = 20, std::uint32_t seed = 0);
    void start(GameMode mode, Difficulty difficulty);
    void backToMenu();
    void tick();
    bool turn(int player, Direction direction);
    void pause();
    void resume();
    int stepIntervalMs() const;
    bool saveSnapshot(const std::string &path, std::string *error = nullptr) const;
    bool loadSnapshot(const std::string &path, std::string *error = nullptr);
    bool loadLeaderboard(const std::string &path, std::string *error = nullptr);
    bool saveLeaderboard(std::string *error = nullptr) const;

    int width() const { return width_; }
    int height() const { return height_; }
    Scene scene() const { return active_->scene(); }
    GameMode mode() const { return mode_; }
    Difficulty difficulty() const { return difficulty_; }
    bool paused() const { return paused_; }
    const Snake &snake(int player) const { return player == 2 ? snake2_ : snake1_; }
    const Food &food() const { return food_; }
    int score(int player) const { return player == 2 ? score2_ : score1_; }
    Winner winner() const { return winner_; }
    EndReason endReason() const { return endReason_; }
    int frames() const { return frames_; }
    const Leaderboard &leaderboard() const { return leaderboard_; }
    void renderScene(const std::function<void(Scene)> &callback) const { active_->render(callback); }
    void setChangeObserver(std::function<void()> observer) { onChange_ = std::move(observer); }

private:
    friend class PlayState;
    void advanceFrame();
    void transition(GameState &next);
    void finish(EndReason reason, bool dead1 = false, bool dead2 = false);
    bool outOfBounds(Cell position) const;
    void resetSnakes();

    int width_;
    int height_;
    std::mt19937 rng_;
    Snake snake1_;
    Snake snake2_;
    Food food_;
    int score1_ = 0;
    int score2_ = 0;
    int frames_ = 0;
    GameMode mode_ = GameMode::Single;
    Difficulty difficulty_ = Difficulty::Normal;
    EndReason endReason_ = EndReason::None;
    Winner winner_ = Winner::None;
    bool paused_ = false;
    Leaderboard leaderboard_;
    std::string leaderboardPath_;
    MenuState menuState_;
    PlayState playState_;
    GameOverState gameOverState_;
    GameState *active_ = &menuState_;
    std::function<void()> onChange_;
    void notifyChanged() const { if (onChange_) onChange_(); }
};
#endif
