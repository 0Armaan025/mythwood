#pragma once

#include "./MainMenu.h"
#include "./Window.h"
#include "./World.h"
#include <stdexcept>

class Game {
public:
  bool init();
  void run();

private:
  Window window;
  MainMenu mainMenu;
  World world;
  bool playing = false;
};
