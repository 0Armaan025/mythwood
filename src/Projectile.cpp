#include "../include/Projectile.h"
#include <SDL2/SDL_render.h>
#include <cmath>

namespace {
void drawFilledCircle(SDL_Renderer *renderer, int centerX, int centerY,
                      int radius, SDL_Color color) {
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

  for (int dy = -radius; dy <= radius; ++dy) {
    const int halfWidth =
        static_cast<int>(std::sqrt(radius * radius - dy * dy));

    // SO UH YEAH WE NEED GUNS
    //& bullets ofc
    SDL_RenderDrawLine(renderer, centerX - halfWidth, centerY + dy,
                       centerX + halfWidth, centerY + dy);
  }
}
} // namespace

Projectile::Projectile(float startX, float startY, float targetX, float targetY)
    : x(startX), y(startY), velocityX(0.0f), velocityY(0.0f) {
  const float dx = targetX - startX;
  const float dy = targetY - startY;
  const float length = std::sqrt(dx * dx + dy * dy);

  if (length < 0.001f) {
    alive = false;
    return;
  }

  velocityX = (dx / length) * speed;
  velocityY = (dy / length) * speed;
}

void Projectile::update(float deltaTime) {
  if (!alive) {
    return;
  }

  x += velocityX * deltaTime;
  y += velocityY * deltaTime;
}

void Projectile::render(SDL_Renderer *renderer, float cameraX,
                        float cameraY) const {
  if (!alive) {
    return;
  }

  SDL_BlendMode previusBlendMode;
  SDL_GetRenderDrawBlendMode(renderer, &previusBlendMode);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  const int centerX = static_cast<int>(x - cameraX);
  const int centerY = static_cast<int>(y - cameraY);

  drawFilledCircle(renderer, centerX, centerY, 12, {100, 65, 255, 44});
  drawFilledCircle(renderer, centerX, centerY, 8, {145, 105, 255, 110});

  drawFilledCircle(renderer, centerX, centerY, 2, {245, 235, 255, 255});

  SDL_SetRenderDrawBlendMode(renderer, previusBlendMode);
}

bool Projectile::isAlive() const { return alive; }

SDL_Rect Projectile::getBounds() const {
  return {static_cast<int>(x) - radius, static_cast<int>(y) - radius,
          radius * 2, radius * 2};
}

void Projectile::destroy() { alive = false; }
