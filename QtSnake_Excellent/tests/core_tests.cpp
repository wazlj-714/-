// core_tests.cpp：独立于 Qt 的业务回归测试。
#include "../core/game.h"
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {
const std::string TMP = (std::filesystem::temp_directory_path() / "qt_snake_test_").string();
int passes = 0;
#define CHECK(COND) do { if (!(COND)) { std::cerr << "FAIL " << __LINE__ << ": " #COND "\n"; std::exit(2); } ++passes; } while (0)

// 通过公开读档接口构造确定场景，无需暴露业务对象内部数据。
std::string createSave(const std::string &name,
                       const std::vector<Cell> &body1, int dir1,
                       const std::vector<Cell> &body2, int dir2,
                       Cell food, int score1 = 0, int score2 = 0,
                       int mode = 1) {
    const std::string path = TMP + name;
    std::ofstream out(path);
    out << "SNAKE_SAVE_V1\n20 20\n" << mode << " 1 " << score1 << " " << score2 << " 0\n";
    out << dir1 << " " << dir1 << " " << body1.size() << "\n";
    for (auto c : body1) out << c.x << " " << c.y << "\n";
    out << dir2 << " " << dir2 << " " << body2.size() << "\n";
    for (auto c : body2) out << c.x << " " << c.y << "\n";
    out << food.x << " " << food.y << "\n";
    return path;
}
void loadAndResume(Game &g, const std::string &path) {
    std::string error;
    CHECK(g.loadSnapshot(path, &error));
    CHECK(g.paused());
    g.resume();
}
// 普通移动、增长和同回合快速反向限制。
void testMovementAndTurns() {
    Snake s;
    s.reset({{5,5},{4,5},{3,5}}, Direction::Right);
    CHECK(s.size() == 3);
    CHECK(s.proposedHead() == (Cell{6,5}));
    CHECK(!s.requestTurn(Direction::Left));
    CHECK(s.requestTurn(Direction::Up));
    CHECK(!s.requestTurn(Direction::Left)); // 快速连续输入不能反向。
    CHECK(s.proposedHead() == (Cell{5,4}));
    s.advance(s.proposedHead(), false);
    CHECK(s.body().front() == (Cell{5,4}));
    CHECK(s.size() == 3);
    CHECK(s.direction() == Direction::Up);
    CHECK(s.requestTurn(Direction::Left));
    s.advance(s.proposedHead(), true);
    CHECK(s.size() == 4);
}
// 开局数据、食物合法位置、暂停继续和菜单操作限制。
void testStartAndFood() {
    Game g(20, 20, 1234);
    CHECK(g.scene() == Scene::Menu);
    g.start(GameMode::Single, Difficulty::Normal);
    CHECK(g.scene() == Scene::Playing);
    CHECK(g.snake(1).size() == 4);
    CHECK(g.snake(2).empty());
    CHECK(!g.snake(1).contains(g.food().position()));
    CHECK(g.food().present());
    CHECK(g.food().position().x >= 0 && g.food().position().x < 20);
    CHECK(g.food().position().y >= 0 && g.food().position().y < 20);
    CHECK(g.stepIntervalMs() == 170);
    const auto old = g.snake(1).body().front();
    g.pause(); g.tick();
    CHECK(g.snake(1).body().front() == old);
    CHECK(!g.turn(1, Direction::Up));
    g.resume();
    CHECK(g.turn(1, Direction::Up));
    g.tick();
    CHECK(g.snake(1).body().front() == (Cell{old.x, old.y - 1}));
    CHECK(g.frames() == 1);
    g.backToMenu();
    CHECK(g.scene() == Scene::Menu);
    CHECK(!g.turn(1, Direction::Left));
}
// 越界、自身碰撞以及结束后停止推进。
void testWallAndBody() {
    Game g(20, 20, 2);
    loadAndResume(g, createSave("wall", {{0,5},{1,5},{2,5}}, 2, {}, 3, {10,10}));
    g.tick();
    CHECK(g.scene() == Scene::GameOver);
    CHECK(g.endReason() == EndReason::Wall);
    CHECK(g.winner() == Winner::None);
    const int frame = g.frames(); g.tick();
    CHECK(g.frames() == frame);
    loadAndResume(g, createSave("body", {{3,3},{3,2},{2,2},{2,3},{2,4}}, 2,
                                 {}, 3, {10,10}));
    g.tick();
    CHECK(g.scene() == Scene::GameOver);
    CHECK(g.endReason() == EndReason::Body);
}
// 释放尾格可进入，进食增长并正确计分。
void testTailAndEating() {
    Game g(20, 20, 5);
    loadAndResume(g, createSave("tail", {{3,3},{3,2},{2,2},{2,3}}, 2,
                                 {}, 3, {10,10}));
    g.tick();
    CHECK(g.scene() == Scene::Playing);
    CHECK(g.snake(1).body().front() == (Cell{2,3}));
    loadAndResume(g, createSave("eat", {{3,3},{2,3},{1,3}}, 3,
                                 {}, 3, {4,3}));
    const auto previous = g.snake(1).size();
    g.tick();
    CHECK(g.score(1) == 10);
    CHECK(g.snake(1).size() == previous + 1);
    CHECK(g.snake(1).body().front() == (Cell{4,3}));
    CHECK(!g.snake(1).contains(g.food().position()));
    CHECK(g.scene() == Scene::Playing);
}
// 双人同格、单方死亡和交换蛇头的同步结算。
void testTwoPlayer() {
    Game g(20, 20, 18);
    g.start(GameMode::Dual, Difficulty::Hard);
    CHECK(!g.snake(2).empty());
    CHECK(g.stepIntervalMs() == 100);
    loadAndResume(g, createSave("headon", {{4,5},{3,5}}, 3,
                                 {{6,5},{7,5}}, 2, {10,10}, 0, 0, 2));
    g.tick();
    CHECK(g.scene() == Scene::GameOver);
    CHECK(g.winner() == Winner::Draw);
    CHECK(g.endReason() == EndReason::HeadOn);
    loadAndResume(g, createSave("one_dead", {{0,5},{1,5}}, 2,
                                 {{10,10},{9,10}}, 3, {15,15}, 0, 0, 2));
    g.tick();
    CHECK(g.winner() == Winner::Player2);
    loadAndResume(g, createSave("swap", {{4,5},{3,5}}, 3,
                                 {{5,5},{6,5}}, 2, {10,10}, 0, 0, 2));
    g.tick();
    CHECK(g.winner() == Winner::Draw);
    CHECK(g.endReason() == EndReason::Body);
}
// 有效存档往返恢复，以及损坏、缺失和重叠数据的拒绝。
void testSerialization() {
    Game g(20,20,123);
    g.start(GameMode::Dual, Difficulty::Hard);
    g.pause();
    const std::string path = TMP + "snapshot_valid";
    std::string error;
    CHECK(g.saveSnapshot(path, &error));
    Game loaded(20,20,999);
    CHECK(loaded.loadSnapshot(path, &error));
    CHECK(loaded.scene() == Scene::Playing);
    CHECK(loaded.paused());
    CHECK(loaded.mode() == GameMode::Dual);
    CHECK(loaded.difficulty() == Difficulty::Hard);
    CHECK(loaded.snake(1).body() == g.snake(1).body());
    CHECK(loaded.snake(2).body() == g.snake(2).body());
    CHECK(loaded.food().position() == g.food().position());
    { std::ofstream out(TMP + "bad"); out << "garbage"; }
    CHECK(!loaded.loadSnapshot(TMP + "bad", &error));
    CHECK(loaded.mode() == GameMode::Dual);
    CHECK(loaded.snake(1).body() == g.snake(1).body());
    CHECK(!loaded.loadSnapshot(TMP + "nonexistent", &error));
    CHECK(!g.turn(3, Direction::Right));
    g.backToMenu();
    CHECK(!g.saveSnapshot(TMP + "unavailable", &error));
    const std::string overlap = createSave("bad_overlap", {{3,3},{2,3},{1,3}}, 3,
                                         {{4,4},{3,4}}, 2, {3,3}, 0, 0, 2);
    CHECK(!loaded.loadSnapshot(overlap, &error));
}
// 排行榜降序、同分保留、数量限制和异常文件保护。
void testScoresAndTop10() {
    Leaderboard r;
    for (int i = 0; i < 12; ++i)
        r.add({(i % 2 == 0 ? i : i-1) * 10, "P1", GameMode::Single, Difficulty::Normal});
    CHECK(r.top10().size() == 10);
    CHECK(r.top10().front().score == 100);
    CHECK(r.top10()[0].score == r.top10()[1].score); // 同分记录均保留。
    const std::string file = TMP + "ranking";
    std::string error;
    CHECK(r.save(file, &error));
    Leaderboard r2;
    CHECK(r2.load(file, &error));
    CHECK(r2.top10().size() == 10);
    CHECK(r2.top10().front().score == 100);
    { std::ofstream f(file); f << "SNAKE_TOP10_V1\n1\n-4 1 1 \"P1\"\n"; }
    CHECK(!r2.load(file, &error));
    CHECK(r2.top10().size() == 10); // 损坏文件不覆盖已有排行榜。
    CHECK(r2.load(TMP + "missing_rankings", &error));
    CHECK(r2.top10().empty());
}
// 满盘、唯一空格和随机食物避开蛇体。
void testFoodFullBoard() {
    std::deque<Cell> cells;
    for (int y = 0; y < 8; ++y) {
        if (y % 2 == 0) for (int x = 0; x < 8; ++x) cells.push_back({x,y});
        else for (int x = 7; x >= 0; --x) cells.push_back({x,y});
    }
    Snake s1, s2;
    s1.reset(cells, Direction::Right);
    Food f;
    std::mt19937 rand(44);
    CHECK(!f.generate(8,8,s1,s2,rand));
    CHECK(!f.present());
    cells.pop_back();
    s1.reset(cells, Direction::Right);
    CHECK(f.generate(8,8,s1,s2,rand));
    CHECK(f.position() == (Cell{0,7}));
    // 使用多组种子抽样检查食物避开蛇体；并非穷举随机数空间。
    for (int i = 0; i < 400; ++i) {
        std::mt19937 seed(i + 1);
        CHECK(f.generate(20,20,s1,s2,seed));
        CHECK(!s1.contains(f.position()));
    }
}
// 多组小棋盘初始化，检查蛇体坐标和食物位置。
void testSmallBoardInitialPositions() {
    for (int width : {8, 9, 10, 12, 16, 20}) {
        for (int height : {8, 10, 20}) {
            Game g(width, height, 35);
            g.start(GameMode::Dual, Difficulty::Easy);
            CHECK(g.scene() == Scene::Playing);
            CHECK(g.snake(1).size() >= 2);
            CHECK(g.snake(2).size() >= 2);
            for (int player : {1, 2}) {
                for (auto c : g.snake(player).body()) {
                    CHECK(c.x >= 0 && c.x < g.width());
                    CHECK(c.y >= 0 && c.y < g.height());
                }
            }
            CHECK(!g.snake(1).contains(g.food().position()));
            CHECK(!g.snake(2).contains(g.food().position()));
        }
    }
}
// 速度下限、暂停继续和纯场景回调。
void testVaryingSpeedAndRender() {
    Game g(20,20,1);
    g.start(GameMode::Single, Difficulty::Easy);
    CHECK(g.stepIntervalMs() == 260);
    bool called = false;
    g.renderScene([&](Scene scene) { called = scene == Scene::Playing; });
    CHECK(called);
    loadAndResume(g, createSave("highscore", {{3,3},{2,3}}, 3,
                                 {}, 3, {10,10}, 500));
    CHECK(g.stepIntervalMs() == 80); // 读入普通难度，高分时受最低移动间隔限制。
    CHECK(g.scene() == Scene::Playing);
    g.pause();
    CHECK(g.paused());
    g.resume();
    CHECK(!g.paused());
}
}
int main() {
    testMovementAndTurns();
    testStartAndFood();
    testWallAndBody();
    testTailAndEating();
    testTwoPlayer();
    testSerialization();
    testScoresAndTop10();
    testFoodFullBoard();
    testVaryingSpeedAndRender();
    testSmallBoardInitialPositions();
    std::cout << "PASS: " << passes << " core assertions (Qt-free)\n";
}
