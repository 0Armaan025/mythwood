#pragma once

#include "./MainMenu.h"
#include "./Window.h"

class Game {
public:
  bool init();
  void run();

private:
  Window window;
  MainMenu mainMenu;
};
