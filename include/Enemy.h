#pragma once

#include <SDL2/SDL.h>

// and ofc we gonna add a cool looking slime, which would be just a good
// harmless slime :)
class Enemy {
public:
  Enemy(float x, float y, SDL_Color color = SDL_Color{85, 205, 110, 255});

  void update(float deltaTime, const SDL_Rect &playerBounds);
  void render(SDL_Renderer *renderer, float cameraX = 0.0f,
              float cameraY = 0.0f) const;

  void takeDamage(int damage);

  SDL_Rect getBounds() const;
  bool isAlive() const;

  void applyKnockback(float originX, float originY, float force);
  int getHealth() const;
  SDL_Color getColor() const;

private:
  float x;
  float y;

  int health = 30;
  int maxHealth = 30;

  float hitFlashTimer = 0.0f;
  bool alive = true;

  float moveSpeed = 75.0f;
  static constexpr float AGGRO_RANGE = 220.0f;
  static constexpr float STOP_DISTANCE = 30.0f;

  float knockbackVX = 0.0f;
  float knockbackVY = 0.0f;

  SDL_Color color;
};
