#pragma once

#include "./Player.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

class World {
public:
  ~World(); // ewww

  bool init();
  void start();
  void update(float deltaTime);
  void render(SDL_Renderer *renderer) const;
  bool shouldReturnToMenu() const; // yay at least this has been done :P

private:
  Player player;
  TTF_Font *font = nullptr;
  bool returnToMenu = false;
};
