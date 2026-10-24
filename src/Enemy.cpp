#include "../include/Enemy.h"
#include <cmath>

#include <algorithm>

Enemy::Enemy(float startX, float startY) : x(startX), y(startY) {}

void Enemy::update(float deltaTime, const SDL_Rect &playerBounds) {
  if (!alive)
    return;

  hitFlashTimer = std::max(0.0f, hitFlashTimer - deltaTime);

  const SDL_Rect enemyBounds = getBounds();

  const float enemyCenterX = enemyBounds.x + enemyBounds.w / 2.0f;

  const float enemyCenterY = enemyBounds.y + enemyBounds.h / 2.0f;

  const float playerCenterX = playerBounds.x + playerBounds.w / 2.0f;

  const float playerCenterY = playerBounds.y + playerBounds.h / 2.0f;

  const float dx = playerCenterX - enemyCenterX;

  const float dy = playerCenterY - enemyCenterY;

  const float distance = std::sqrt(dx * dx + dy * dy);

  if (distance > AGGRO_RANGE || distance <= STOP_DISTANCE) {
    return;
  }

  if (distance > 0.0f) {
    x += (dx / distance) * moveSpeed * deltaTime;
    y += (dy / distance) * moveSpeed * deltaTime;
  }
}

void Enemy::render(SDL_Renderer *renderer, float cameraX, float cameraY) const {
  if (!alive)
    return;

  const SDL_Rect body = getBounds();

  SDL_Rect screenBody = body;
  screenBody.x -= static_cast<int>(cameraX);
  screenBody.y -= static_cast<int>(cameraY);
  if (hitFlashTimer > 0.0f)
    SDL_SetRenderDrawColor(renderer, 255, 245, 245, 255);

  else {
    SDL_SetRenderDrawColor(renderer, 85, 205, 110, 255);
  }

  SDL_Rect slimeBody = {screenBody.x + 2, screenBody.y + 8, screenBody.w - 4,
                        screenBody.h - 8};

  SDL_RenderFillRect(renderer, &slimeBody);

  // eyes

  SDL_SetRenderDrawColor(renderer, 25, 45, 35, 255);
  SDL_Rect leftEye = {screenBody.x + 7, screenBody.y + 13, 4, 5};
  SDL_Rect rightEye = {screenBody.x + 17, screenBody.y + 13, 4, 5};

  SDL_RenderFillRect(renderer, &leftEye);
  SDL_RenderFillRect(renderer, &rightEye);

  const int barWidth = 32;
  const int barHeight = 5;

  const int barX = screenBody.x + (screenBody.w - barWidth) / 2;
  const int barY = screenBody.y - 2;

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
