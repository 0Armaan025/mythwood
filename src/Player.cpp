// ALRIGHT SO I DON'T HAVE SPRITES BUT IM GONNA USE MY TURTLE SKILLS IN CPP

#include "../include/Player.h"
#include <algorithm>
#include <iostream>

SDL_Rect Player::getBounds() const {
  return {static_cast<int>(x), static_cast<int>(y), 24, 32};
}

void Player::update(float deltaTime, const std::vector<SDL_Rect> &obstacles) {
  const Uint8 *keys = SDL_GetKeyboardState(nullptr);

  float dx = 0.0f;
  float dy = 0.0f;

  if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
    dy -= 1;
  if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
    dy += 1;
  if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])
    dx -= 1;
  if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])
    dx += 1;

  // for diagonal stuff ig
  if (dx != 0 && dy != 0) {
    dx *= 0.70710678f;
    dy *= 0.70710678f;
  }

  const float moveX = dx * speed * deltaTime;
  const float moveY = dy * speed * deltaTime;

  x = std::clamp(x + moveX, 0.0f, 936.0f);

  SDL_Rect candidate = getBounds();
  bool blocked = false;

  for (const SDL_Rect &obstacle : obstacles) {
    if (SDL_HasIntersection(&candidate, &obstacle)) {
      cout << "collision" << endl;
      blocked = true;
      break;
    }
  }

  if (blocked) {
    x = x - moveX;
  }

  // vertical wall slide

  y = std::clamp(y + moveY, 76.0f, 600.0f);
  candidate = getBounds();
  blocked = false;
  for (const SDL_Rect &obstacle : obstacles) {
    if (SDL_HasIntersection(&candidate, &obstacle)) {
      blocked = true;
      break;
    }
  }

  if (blocked)
    y -= moveY;
}

void Player::render(SDL_Renderer *renderer) const {

  const SDL_Rect body = getBounds();

  // SHADOW (SAMA)

  SDL_SetRenderDrawColor(renderer, 12, 20, 15, 150);
  SDL_Rect shadow{body.x + 3, body.y + 27, 22, 7};
  SDL_RenderFillRect(renderer, &shadow);

  // head

  SDL_SetRenderDrawColor(renderer, 222, 183, 135, 255);
  SDL_Rect head{body.x + 5, body.y, 14, 12};
  SDL_RenderFillRect(renderer, &head);

  // cloak

  SDL_SetRenderDrawColor(renderer, 75, 110, 75, 255);
  SDL_Rect cloak{body.x + 2, body.y + 12, 20, 16};
  SDL_RenderFillRect(renderer, &cloak);

  // Boots
  SDL_SetRenderDrawColor(renderer, 75, 51, 37, 255);
  SDL_Rect boots{body.x + 3, body.y + 28, 7, 4};
  SDL_RenderFillRect(renderer, &boots);

  SDL_Rect boot2{body.x + 14, body.y + 28, 7, 4};
  SDL_RenderFillRect(renderer, &boot2);
}
void Player::reset() {
  x = 480.0f;
  y = 350.0f;
}
