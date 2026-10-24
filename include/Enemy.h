#pragma once

#include <SDL2/SDL.h>

// and ofc we gonna add a cool looking slime, which would be just a good
// harmless slime :)
class Enemy {
public:
  Enemy(float x, float y);

  void update(float deltaTime);
  void render(SDL_Renderer *renderer, float cameraX = 0.0f,
              float cameraY = 0.0f) const;

  void takeDamage(int damage);

  SDL_Rect getBounds() const;
  bool isAlive() const;

  int getHealth() const;

private:
  float x;
  float y;

  int health = 30;
  int maxHealth = 30;

  float hitFlashTimer = 0.0f;
  bool alive = true;
};
