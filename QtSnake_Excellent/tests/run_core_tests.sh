#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
${CXX:-g++} -std=c++17 -O2 -Wall -Wextra -Wpedantic \
 core/snake.cpp core/food.cpp core/leaderboard.cpp core/gamestate.cpp core/game.cpp \
 tests/core_tests.cpp -o tests/core_tests
./tests/core_tests
