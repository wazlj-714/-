// leaderboard.h - Top10 scores are grouped in a descending std::map.
#ifndef SNAKE_LEADERBOARD_H
#define SNAKE_LEADERBOARD_H
#include "types.h"
#include <functional>
#include <map>
#include <string>
#include <vector>
struct ScoreRecord {
    int score = 0;
    std::string player;
    GameMode mode = GameMode::Single;
    Difficulty difficulty = Difficulty::Normal;
};
class Leaderboard {
public:
    void add(const ScoreRecord &record);
    std::vector<ScoreRecord> top10() const;
    bool load(const std::string &path, std::string *error = nullptr);
    bool save(const std::string &path, std::string *error = nullptr) const;
    void clear() { byScore_.clear(); }
private:
    std::map<int, std::vector<ScoreRecord>, std::greater<int>> byScore_;
    void trim();
};
#endif
