# Qt 贪吃蛇 · 面向对象综合实训（改进版）

本工程基于用户上传的 `demo_snack2.pro / main.cpp / widget.* / widget.ui` 的单人、双人贪吃蛇功能重新组织，保留玩法并将原本集中于 `Widget` 的游戏规则拆到纯 C++17 业务模块。

## 一、运行

1. 在 Qt Creator 打开 **`demo_snack2.pro`**（推荐用户原有 **Desktop Qt 6.6.2 MinGW 64-bit**；亦支持 Qt 5.15+），选择 Desktop Kit。
2. 点击 **构建** → **运行**。请注意不要把原工程的 `.pro.user` 配置复制到不同电脑，它包含本机路径，Qt Creator 会自动生成。
3. 双人模式 P1 使用 WASD，P2 使用方向键；单人使用方向键。

## 二、功能

- 单人和双人同步回合；禁止 180° 反向；边界、自身、他蛇、双方同格碰撞；移动到即将腾空的尾格不算碰撞。
- 菜单 / 游玩 / 结束三态，多态 `GameState`，运行状态可暂停；分数和动态速度。
- 简单、普通、困难三档难度；三款策略式视觉皮肤（仅 GUI 层，独立于规则）。
- `std::deque` 存蛇身；`std::map` 存 Top10（正确保留同分玩家）。
- 排行榜自动本地持久化；三存档位可随时保存、读取，解析校验后才修改当前游戏；读档默认暂停。
- 核心业务头文件无 Qt 依赖，可独立以 g++/clang++ 编译并自动化测试。

## 三、按钮和热键

| 行为 | 按键/按钮 |
|---|---|
| 单人 / 双人 | `1` / `2` 或界面对应按钮 |
| 暂停 / 继续 | `P`（或空格） / `C` |
| 返回主菜单 | `Esc` |
| 保存 / 读取 | 选择存档位后点击按钮；`Ctrl+S` / `Ctrl+L`（游戏中） |
| 结束后重开 | `R`（保持本局模式） |
| 退出 | 「退出程序」按钮 |

说明：游戏逻辑均为整格移动，而非帧插值；最高分与存档保存于 Qt 标准 AppDataLocation 对应的应用数据目录（非源代码目录）。

## 四、独立测试

Linux / macOS：`bash tests/run_core_tests.sh`（只需要 g++ / clang++）

Windows Qt Creator 自带的 MinGW 终端（位于工程根目录）：

```
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic core/snake.cpp core/food.cpp core/leaderboard.cpp core/gamestate.cpp core/game.cpp tests/core_tests.cpp -o core_tests.exe
core_tests.exe
```

测试使用 C++17 的 `std::filesystem::temp_directory_path()` 自动取得当前平台的临时目录，适用于 Linux/macOS/Windows。GUI 本身与测试使用同一核心代码。

## 五、文件与分层

- 业务层：`core/types.h / snake.* / food.* / game.* / gamestate.* / leaderboard.*`。
- 视图层/交互层：`widget.* / widget.ui / skinstrategy.* / main.cpp`。
- `tests/core_tests.cpp`：可重复执行的核心回归测试；`diagrams/*.mmd`：可编辑 Mermaid 图。

## 六、注意

- 提交前请补齐 Word 报告中的姓名、班级、学号和教师姓名，再在你自己的 Qt Creator 里编译与截图；自动化测试不能代替真实 GUI 手工验收。
- 本版本未包含 Qt Multimedia 音效（该项在任务书中属于附加挑战，不影响已实现的皮肤、存档挑战）。
