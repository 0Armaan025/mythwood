#include "../include/Game.h"

bool Game::init() { return window.create("Mythwood", 960, 640); }

void Game::run() {
  while (window.isRunning()) {
    window.pollEvents();

    window.clear();

    // my stuff
    window.present();
  }
}
