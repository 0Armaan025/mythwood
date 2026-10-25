#include "../include/Rocket.h"
#include <algorithm>
#include <cmath>

namespace {
void drawFilledCircle(SDL_Renderer *renderer, int centerX, int centerY,
                      int radius, SDL_Color color) {

  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

  for (int dy = -radius; dy <= radius; ++dy) {
    const int width = static_cast<int>(
        std::sqrt(static_cast<float>(radius * radius - dy * dy)));

    SDL_RenderDrawLine(renderer, centerX - width, centerY + dy, centerX + width,
                       centerY + dy);
  }
}
} // namespace

Rocket::Rocket(float startX, float startY, float targetX, float targetY)
    : x(startX), y(startY) {

  // hi
  //

  const float dx = targetX - startX;
  const float dy = targetY - startY;

  const float length = std::sqrt(dx * dx + dy * dy);

  if (length < 0.001f) {
    alive = false;
    return;
  }

  velocityX = (dx / length) * SPEED;
  velocityY = (dy / length) * SPEED;
}

void Rocket::update(float deltaTime) {
  // alright

  if (!alive) {
    return;
  }

  if (exploding) {
    explosionTimer -= deltaTime;

    if (explosionTimer <= 0.0f) {
      alive = false;
    }

    return;
  }

  x += velocityX * deltaTime;
  y += velocityY * deltaTime;
  flightTime += deltaTime;

  if (flightTime >= MAX_FLIGHT_TIME) {
    beginExplosion();
  }
}

void Rocket::render(SDL_Renderer *renderer, float cameraX,
                    float cameraY) const {
  if (!alive) {
    return;
  }

  SDL_BlendMode previousBlendMode;
  SDL_GetRenderDrawBlendMode(renderer, &previousBlendMode);

  const int screenX = static_cast<int>(x - cameraX);
  const int screenY = static_cast<int>(y - cameraY);

  if (exploding) {
    const float progress =
        std::clamp(1.0f - explosionTimer / EXPLOSION_DURATION, 0.0f, 1.0f);

    const int radius = static_cast<int>(BLAST_RADIUS * progress);

    drawFilledCircle(renderer, screenX, screenY, radius, {255, 70, 15, 65});

    drawFilledCircle(renderer, screenX, screenY, std::max(2, radius * 2 / 3),
                     {255, 155, 25, 110});

    drawFilledCircle(renderer, screenX, screenY, std::max(2, radius / 3),
                     {255, 235, 145, 210});
  }

  else {
    /// cool rocket here

    drawFilledCircle(renderer, screenX, screenY, 10, {255, 40, 10, 90});

    drawFilledCircle(renderer, screenX, screenY, 6, {255, 115, 20, 255});

    drawFilledCircle(renderer, screenX, screenY, 3, {255, 245, 155, 255});

    const int tailX = screenX - static_cast<int>(velocityX * 0.035f);

    const int tailY = screenY - static_cast<int>(velocityY * 0.035f);

    SDL_SetRenderDrawColor(renderer, 255, 100, 20, 230);

    SDL_RenderDrawLine(renderer, screenX, screenY, tailX, tailY);
  }

  SDL_SetRenderDrawBlendMode(renderer, previousBlendMode);
}

bool Rocket::isAlive() const { return alive; }

bool Rocket::isFlying() const { return alive && !exploding; }

bool Rocket::isExploding() const { return alive && exploding; }

SDL_Rect Rocket::getBounds() const {
  return {static_cast<int>(x) - 7, static_cast<int>(y) - 7, 14, 14};
}

float Rocket::getCenterX() const { return x; }

float Rocket::getCenterY() const { return y; }

void Rocket::beginExplosion() {
  if (!alive || exploding) {
    return;
  }

  exploding = true;
  explosionTimer = EXPLOSION_DURATION;
  velocityX = 0.0f;
  velocityY = 0.0f;
}

void Rocket::destroy() { alive = false; }
