#pragma once

#include <SDL2/SDL.h>

class Rocket {
public:
  Rocket(float startX, float startY, float targetX, float targetY);

  void update(float deltaTime);

  void render(SDL_Renderer *renderer, float cameraX = 0.0f,
              float cameraY = 0.0f) const;

  bool isAlive() const;
  bool isFlying() const;
  bool isExploding() const;

  void beginExplosion();
  void destroy();
  SDL_Rect getBounds() const;

  float getCenterX() const;
  float getCenterY() const;

  static constexpr float BLAST_RADIUS = 100.0f;
  static constexpr int BLAST_DAMAGE = 45;
  static constexpr float KNOCKBACK_FORCE = 460.0f;

private:
  float x;
  float y;

  float velocityX = 0.0f;
  float velocityY = 0.0f;

  // UHHH IM TIREEDD
  float flightTime = 0.0f;
  float explosionTimer = 0.0f;

  bool alive = true;
  bool exploding = false;

  static constexpr float SPEED = 560.0f;
  static constexpr float MAX_FLIGHT_TIME = 2.0f;
  static constexpr float EXPLOSION_DURATION = 0.30f;
};
