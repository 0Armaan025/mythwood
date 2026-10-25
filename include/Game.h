#pragma once

#include "./MainMenu.h"
#include "./Window.h"
#include "./World.h"
#include <SDL2/SDL.h>
#include <stdexcept>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

class Game {
public:
  bool init();
  void run();

private:
  Window window;
  MainMenu mainMenu;
  World world;
  bool playing = false;
  Uint64 previousCounter = 0;

  bool runFrame();

#ifdef __EMSCRIPTEN__
  static void mainLoopCallback(void *arg);
#endif
};
