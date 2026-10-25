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

bool Game::runFrame() {
  Uint64 current = SDL_GetPerformanceCounter();

  float deltaTime =
      static_cast<float>(static_cast<double>(current - previousCounter) /
                         SDL_GetPerformanceFrequency());

  previousCounter = current;

  if (deltaTime > 0.05f)
    deltaTime = 0.05f;

  window.pollEvents();

  if (!window.isRunning())
    return false;

  if (!playing) {
    if (!mainMenu.update())
      return false;

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

  return true;
}

void Game::run() {
  previousCounter = SDL_GetPerformanceCounter();

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(mainLoopCallback, this, 0, 1);
#else
  while (window.isRunning() && runFrame())
    ;
#endif
}

#ifdef __EMSCRIPTEN__
void Game::mainLoopCallback(void *arg) {
  if (!static_cast<Game *>(arg)->runFrame()) {
    emscripten_cancel_main_loop();
  }
}
#endif
