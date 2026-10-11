// gamestate.cpp：运行状态通过 Game 推进回合，规则仍集中在 Game 中。
#include "gamestate.h"
#include "game.h"
void PlayState::update(Game &game) { game.advanceFrame(); }
