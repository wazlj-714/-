// leaderboard.cpp - Handles tied scores, malformed input, and Top10 persistence.
#include "leaderboard.h"
#include <fstream>
#include <iomanip>
#include <sstream>

void Leaderboard::add(const ScoreRecord &record) {
    if (record.score < 0 || record.score > 1000000 || record.player.empty() ||
        record.player.size() > 32) return;
    byScore_[record.score].push_back(record);
    trim();
}
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
void Leaderboard::trim() {
    const auto entries = top10();
    byScore_.clear();
    for (const auto &entry : entries) byScore_[entry.score].push_back(entry);
}
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
    if (!input) { // Missing file is normal on first run.
        clear();
        return true;
    }
    std::string magic;
    int count = -1;
    if (!(input >> magic >> count) || magic != "SNAKE_TOP10_V1" || count < 0 || count > 10) {
        if (error) *error = "Invalid leaderboard header";
        return false;
    }
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
    // No trailing garbage may masquerade as a successful load.
    std::string unexpected;
    if (input >> unexpected) {
        if (error) *error = "Unexpected leaderboard content";
        return false;
    }
    *this = std::move(parsed);
    return true;
}
