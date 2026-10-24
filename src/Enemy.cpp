#include "../include/Enemy.h"

#include <algorithm>

Enemy::Enemy(float startX, float startY) : x(startX), y(startY) {}

void Enemy::update(float deltaTime) {
  if (!alive)
    return;

  hitFlashTimer = std::max(0.0f, hitFlashTimer - deltaTime);
}

void Enemy::render(SDL_Renderer *renderer) const {
  if (!alive)
    return;

  const SDL_Rect body = getBounds();

  if (hitFlashTimer > 0.0f)
    SDL_SetRenderDrawColor(renderer, 255, 245, 245, 255);

  else {
    SDL_SetRenderDrawColor(renderer, 85, 205, 110, 255);
  }

  SDL_Rect slimeBody = {body.x + 2, body.y + 8, body.w - 4, body.h - 8};

  SDL_RenderFillRect(renderer, &slimeBody);

  // eyes

  SDL_SetRenderDrawColor(renderer, 25, 45, 35, 255);
  SDL_Rect leftEye = {body.x + 7, body.y + 13, 4, 5};
  SDL_Rect rightEye = {body.x + 17, body.y + 13, 4, 5};

  SDL_RenderFillRect(renderer, &leftEye);
  SDL_RenderFillRect(renderer, &rightEye);

  const int barWidth = 32;
  const int barHeight = 5;

  const int barX = body.x + (body.w - barWidth) / 2;
  const int barY = body.y - 2;

  // bg

  SDL_SetRenderDrawColor(renderer, 35, 25, 35, 230);

  SDL_Rect barBG = {barX, barY, barWidth, barHeight};

  SDL_RenderFillRect(renderer, &barBG);

  const float healthRatio = static_cast<float>(health) / maxHealth;

  SDL_SetRenderDrawColor(renderer, 220, 55, 65, 255);

  SDL_Rect barForeground = {barX + 1, barY + 1,
                            static_cast<int>((barWidth - 2) * healthRatio),
                            barHeight - 2};

  SDL_RenderFillRect(renderer, &barForeground);
}

void Enemy::takeDamage(int damage) {
  if (!alive || damage <= 0) {
    return;
  }

  health = std::max(0, health - damage);

  hitFlashTimer = 0.12f;

  if (health == 0) {
    alive = false;
  }
}

SDL_Rect Enemy::getBounds() const {
  return {static_cast<int>(x), static_cast<int>(y), 28, 24};
}

bool Enemy::isAlive() const { return alive; }

int Enemy::getHealth() const { return health; }
