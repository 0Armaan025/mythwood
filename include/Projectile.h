#pragma once
#include <SDL2/SDL.h>

class Projectile {
public:
  Projectile(float startX, float startY, float targetX, float targetY);

  void update(float deltaTime);
  void render(SDL_Renderer *renderer, float cameraX = 0.0f,
              float cameraY = 0.0f) const;

  bool isAlive() const;
  SDL_Rect getBounds() const;
  void destroy();

private:
  float x;
  float y;
  float velocityX;
  float velocityY;
  float speed = 420.0f;
  int radius = 6;
  bool alive = true;
};
