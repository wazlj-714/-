// core_tests.cpp - Native C++17 regression suite independent of Qt.
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

// Test fixtures use the public load interface rather than adding privileged debug methods.
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
void testMovementAndTurns() {
    Snake s;
    s.reset({{5,5},{4,5},{3,5}}, Direction::Right);
    CHECK(s.size() == 3);
    CHECK(s.proposedHead() == (Cell{6,5}));
    CHECK(!s.requestTurn(Direction::Left));
    CHECK(s.requestTurn(Direction::Up));
    CHECK(!s.requestTurn(Direction::Left)); // rapid input may not reverse committed heading
    CHECK(s.proposedHead() == (Cell{5,4}));
    s.advance(s.proposedHead(), false);
    CHECK(s.body().front() == (Cell{5,4}));
    CHECK(s.size() == 3);
    CHECK(s.direction() == Direction::Up);
    CHECK(s.requestTurn(Direction::Left));
    s.advance(s.proposedHead(), true);
    CHECK(s.size() == 4);
}
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
void testScoresAndTop10() {
    Leaderboard r;
    for (int i = 0; i < 12; ++i)
        r.add({(i % 2 == 0 ? i : i-1) * 10, "P1", GameMode::Single, Difficulty::Normal});
    CHECK(r.top10().size() == 10);
    CHECK(r.top10().front().score == 100);
    CHECK(r.top10()[0].score == r.top10()[1].score); // keep ties
    const std::string file = TMP + "ranking";
    std::string error;
    CHECK(r.save(file, &error));
    Leaderboard r2;
    CHECK(r2.load(file, &error));
    CHECK(r2.top10().size() == 10);
    CHECK(r2.top10().front().score == 100);
    { std::ofstream f(file); f << "SNAKE_TOP10_V1\n1\n-4 1 1 \"P1\"\n"; }
    CHECK(!r2.load(file, &error));
    CHECK(r2.top10().size() == 10); // corrupt file does not wipe previous data
    CHECK(r2.load(TMP + "missing_rankings", &error));
    CHECK(r2.top10().empty());
}
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
    // Exhaustively sample seeds: no food appears on any occupied segment.
    for (int i = 0; i < 400; ++i) {
        std::mt19937 seed(i + 1);
        CHECK(f.generate(20,20,s1,s2,seed));
        CHECK(!s1.contains(f.position()));
    }
}
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
void testVaryingSpeedAndRender() {
    Game g(20,20,1);
    g.start(GameMode::Single, Difficulty::Easy);
    CHECK(g.stepIntervalMs() == 260);
    bool called = false;
    g.renderScene([&](Scene scene) { called = scene == Scene::Playing; });
    CHECK(called);
    loadAndResume(g, createSave("highscore", {{3,3},{2,3}}, 3,
                                 {}, 3, {10,10}, 500));
    CHECK(g.stepIntervalMs() == 80); // loaded Normal difficulty, clamped to 80
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
