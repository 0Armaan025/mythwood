
#include "../include/Game.h"
#include <SDL2/SDL_timer.h>

bool Game::init() {
  if (!window.create("Mythwood", 960, 640))
    return false;

  if (!mainMenu.init())
    return false;

  if (!world.init())
    return false;

  return true;
}

void Game::run() {
  Uint64 previous = SDL_GetPerformanceCounter();
  while (window.isRunning()) {
    Uint64 current = SDL_GetPerformanceCounter();

    // tired
    float deltaTime =
        static_cast<float>(static_cast<double>(current - previous) /
                           SDL_GetPerformanceFrequency());

    previous = current;

    if (deltaTime > 0.05f)
      deltaTime = 0.05f;
    window.pollEvents();

    if (!playing) {
      if (!mainMenu.update())
        break;

      if (mainMenu.takeNewGameRequest()) {
        world.start();
        playing = true;
      }

      if (mainMenu.takeContinueRequest()) {
        if (world.loadGame()) {
          playing = true;
        }
      }

      mainMenu.render(window.getRenderer());
    } else {
      world.update(deltaTime, window.getRenderer());

      if (world.shouldReturnToMenu()) {
        playing = false;
      } else {
        world.render(window.getRenderer());
      }
    }
  }
}
