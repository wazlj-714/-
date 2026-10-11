// leaderboard.h：按分数降序分组保存记录，输出前十名。
#ifndef SNAKE_LEADERBOARD_H
#define SNAKE_LEADERBOARD_H
#include "types.h"
#include <functional>
#include <map>
#include <string>
#include <vector>
// 成绩记录只传递数据，公开字段不直接暴露排行榜内部容器。
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
    // map 让分数降序排列，vector 保存同分玩家，避免覆盖。
    std::map<int, std::vector<ScoreRecord>, std::greater<int>> byScore_;
    void trim();
};
#endif
