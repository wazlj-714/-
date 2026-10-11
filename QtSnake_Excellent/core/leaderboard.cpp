// leaderboard.cpp：处理同分记录、文件校验和排行榜持久化。
#include "leaderboard.h"
#include <fstream>
#include <iomanip>
#include <sstream>

// 拒绝异常记录；有效成绩加入对应分数组后裁剪到前十名。
void Leaderboard::add(const ScoreRecord &record) {
    if (record.score < 0 || record.score > 1000000 || record.player.empty() ||
        record.player.size() > 32) return;
    byScore_[record.score].push_back(record);
    trim();
}
// 降序遍历分数组，同分时保持组内插入顺序。
std::vector<ScoreRecord> Leaderboard::top10() const {
    std::vector<ScoreRecord> entries;
    for (const auto &group : byScore_) {
        for (const auto &item : group.second) {
            entries.push_back(item);
            if (entries.size() == 10) return entries;
        }
    }
    return entries;
}
// 用前十条记录重建容器，限制内存中保留的成绩数量。
void Leaderboard::trim() {
    const auto entries = top10();
    byScore_.clear();
    for (const auto &entry : entries) byScore_[entry.score].push_back(entry);
}
// 写入版本标记及记录，并在刷新输出后检查写入结果。
bool Leaderboard::save(const std::string &path, std::string *error) const {
    std::ofstream output(path, std::ios::trunc);
    if (!output) { if (error) *error = "Cannot open leaderboard file"; return false; }
    const auto entries = top10();
    output << "SNAKE_TOP10_V1\n" << entries.size() << '\n';
    for (const auto &e : entries)
        output << e.score << ' ' << static_cast<int>(e.mode) << ' '
               << static_cast<int>(e.difficulty) << ' ' << std::quoted(e.player) << '\n';
    output.flush();
    if (!output) { if (error) *error = "Cannot write leaderboard file"; return false; }
    return true;
}
bool Leaderboard::load(const std::string &path, std::string *error) {
    std::ifstream input(path);
    if (!input) { // 此分支将文件无法打开视为首次运行，清空排行榜后返回成功。
        clear();
        return true;
    }
    std::string magic;
    int count = -1;
    if (!(input >> magic >> count) || magic != "SNAKE_TOP10_V1" || count < 0 || count > 10) {
        if (error) *error = "Invalid leaderboard header";
        return false;
    }
    // 先装入临时排行榜，防止中途读到损坏记录时破坏原数据。
    Leaderboard parsed;
    for (int i = 0; i < count; ++i) {
        ScoreRecord r;
        int mode = 0, difficulty = 0;
        if (!(input >> r.score >> mode >> difficulty >> std::quoted(r.player)) ||
            r.score < 0 || r.score > 1000000 || r.player.empty() || r.player.size() > 32 ||
            (mode != 1 && mode != 2) || difficulty < 0 || difficulty > 2) {
            if (error) *error = "Invalid leaderboard record";
            return false;
        }
        r.mode = static_cast<GameMode>(mode);
        r.difficulty = static_cast<Difficulty>(difficulty);
        parsed.add(r);
    }
    // 拒绝记录后多余的数据，避免把损坏文件当作有效排行榜。
    std::string unexpected;
    if (input >> unexpected) {
        if (error) *error = "Unexpected leaderboard content";
        return false;
    }
    // 全部内容合法后一次性替换已有排行榜。
    *this = std::move(parsed);
    return true;
}
