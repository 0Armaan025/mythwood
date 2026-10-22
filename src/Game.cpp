
#include "../include/Game.h"

bool Game::init() {
  if (!window.create("Mythwood", 960, 640))
    return false;

  return mainMenu.init();
}

void Game::run() {
  while (window.isRunning()) {
    window.pollEvents();

    if (!mainMenu.update())
      break;

    mainMenu.render(window.getRenderer());
  }
}
