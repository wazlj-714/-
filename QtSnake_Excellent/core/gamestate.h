// gamestate.h - Polymorphic state machine, using a pure-data rendering callback.
#ifndef SNAKE_GAMESTATE_H
#define SNAKE_GAMESTATE_H
#include "types.h"
#include <functional>
class Game;
class GameState {
public:
    virtual ~GameState() = default;
    virtual Scene scene() const = 0;
    virtual void onEnter(Game &game) = 0;
    virtual void onExit(Game &game) = 0;
    virtual void update(Game &game) = 0;
    // The callback is supplied by the presentation layer; no GUI types enter this layer.
    virtual void render(const std::function<void(Scene)> &callback) const = 0;
};
class MenuState final : public GameState {
public:
    Scene scene() const override { return Scene::Menu; }
    void onEnter(Game &) override {}
    void onExit(Game &) override {}
    void update(Game &) override {}
    void render(const std::function<void(Scene)> &cb) const override { cb(scene()); }
};
class PlayState final : public GameState {
public:
    Scene scene() const override { return Scene::Playing; }
    void onEnter(Game &) override {}
    void onExit(Game &) override {}
    void update(Game &game) override;
    void render(const std::function<void(Scene)> &cb) const override { cb(scene()); }
};
class GameOverState final : public GameState {
public:
    Scene scene() const override { return Scene::GameOver; }
    void onEnter(Game &) override {}
    void onExit(Game &) override {}
    void update(Game &) override {}
    void render(const std::function<void(Scene)> &cb) const override { cb(scene()); }
};
#endif
