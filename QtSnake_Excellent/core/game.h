// game.h：业务核心，统一管理状态、碰撞、计分和文件数据。
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

// 界面提交操作，Game 统一执行规则；绘制事件只查询结果。
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

    // 只读查询区：蛇、食物和排行榜不向界面暴露可写引用。
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
    // 场景回调只传递枚举，业务层无需认识窗口对象。
    void renderScene(const std::function<void(Scene)> &callback) const { active_->render(callback); }
    // 变化通知不携带界面对象，调用方自行注册刷新行为。
    void setChangeObserver(std::function<void()> observer) { onChange_ = std::move(observer); }

private:
    // 仅运行状态可调用私有回合推进函数。
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
    // 暂停是运行状态内部的标记，没有额外的暂停状态对象。
    bool paused_ = false;
    Leaderboard leaderboard_;
    std::string leaderboardPath_;
    // 三个具体状态均为值成员，其生命周期由 Game 管理。
    MenuState menuState_;
    PlayState playState_;
    GameOverState gameOverState_;
    // 非拥有指针：只引用上述某个状态，不负责创建或释放对象。
    GameState *active_ = &menuState_;
    std::function<void()> onChange_;
    void notifyChanged() const { if (onChange_) onChange_(); }
};
#endif
