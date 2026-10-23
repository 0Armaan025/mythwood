#pragma once
#include <SDL2/SDL.h>

class Player {
public:
  void update(
      float deltaTime); // OH THIS BRINGS BACK NOSTALGIA FROM WHEN I USED UNITY
  void render(SDL_Renderer *renderer) const;
  void reset();

  SDL_Rect getBounds() const;

private:
  float x = 480.0f;
  float y = 350.0f;
  float speed = 220.0f;
};
