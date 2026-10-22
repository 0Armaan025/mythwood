#pragma once

#include "./Window.h"

class Game {
public:
  bool init();
  void run();

private:
  Window window;
};
