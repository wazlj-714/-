// gamestate.cpp - Only the active PlayState drives a simulation frame.
#include "gamestate.h"
#include "game.h"
void PlayState::update(Game &game) { game.advanceFrame(); }
