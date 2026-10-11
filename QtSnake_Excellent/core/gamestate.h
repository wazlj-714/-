// gamestate.h：通过继承多态统一游戏状态，渲染回调只传递场景枚举。
#ifndef SNAKE_GAMESTATE_H
#define SNAKE_GAMESTATE_H
#include "types.h"
#include <functional>
class Game;
// 抽象状态定义统一行为，由当前状态的虚函数决定执行方式。
class GameState {
public:
    virtual ~GameState() = default;
    virtual Scene scene() const = 0;
    virtual void onEnter(Game &game) = 0;
    virtual void onExit(Game &game) = 0;
    virtual void update(Game &game) = 0;
    // 界面提供回调并负责绘制，状态类不接触窗口、画笔或颜色。
    virtual void render(const std::function<void(Scene)> &callback) const = 0;
};
// 菜单只提供场景信息，不推进游戏；进入和退出接口保留为扩展点。
class MenuState final : public GameState {
public:
    Scene scene() const override { return Scene::Menu; }
    void onEnter(Game &) override {}
    void onExit(Game &) override {}
    void update(Game &) override {}
    void render(const std::function<void(Scene)> &cb) const override { cb(scene()); }
};
// 运行状态负责调用业务推进；暂停过滤由 Game 完成。
class PlayState final : public GameState {
public:
    Scene scene() const override { return Scene::Playing; }
    void onEnter(Game &) override {}
    void onExit(Game &) override {}
    void update(Game &game) override;
    void render(const std::function<void(Scene)> &cb) const override { cb(scene()); }
};
// 结束状态保留终局画面，不继续改变蛇体。
class GameOverState final : public GameState {
public:
    Scene scene() const override { return Scene::GameOver; }
    void onEnter(Game &) override {}
    void onExit(Game &) override {}
    void update(Game &) override {}
    void render(const std::function<void(Scene)> &cb) const override { cb(scene()); }
};
#endif
