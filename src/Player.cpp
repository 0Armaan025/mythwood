// ALRIGHT SO I DON'T HAVE SPRITES BUT IM GONNA USE MY TURTLE SKILLS IN CPP

#include "../include/Player.h"
#include <SDL2/SDL_render.h>
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;

SDL_Rect Player::getBounds() const {
  return {static_cast<int>(x), static_cast<int>(y), 24, 32};
}

void Player::update(float deltaTime, const std::vector<SDL_Rect> &obstacles) {

  const float startX = x;
  const float startY = y;

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

  // x = std::clamp(x + moveX, 0.0f, 936.0f);

  x = x + moveX; // new: coz we gotta do infinite gen

  SDL_Rect candidate = getBounds();
  bool blocked = false;

  for (const SDL_Rect &obstacle : obstacles) {
    if (SDL_HasIntersection(&candidate, &obstacle)) {
      blocked = true;
      break;
    }
  }

  if (blocked) {
    x = x - moveX;
  }

  // vertical wall slide

  // y = std::clamp(y + moveY, 76.0f, 600.0f);
  y += moveY;
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

  const float movedX = x - startX;
  const float movedY = y - startY;

  isMoving = (movedX * movedX + movedY * movedY) > 0.01f;

  if (isMoving) {
    walkTime += deltaTime * 10.0f;
  }
}

void Player::render(SDL_Renderer *renderer, float cameraX,
                    float cameraY) const {

  const SDL_Rect body = getBounds();

  SDL_Rect screenBody = body;

  screenBody.x -= static_cast<int>(cameraX);
  screenBody.y -= static_cast<int>(cameraY);

  const float phase = isMoving ? walkTime : 0.0f;

  const int legSwing = isMoving ? static_cast<int>(std::sin(phase) * 3.0f) : 0;

  const int oppositeSwing = -legSwing;

  const int bob =
      isMoving ? static_cast<int>((std::sin(phase * 2.0f) + 1.0f) * 0.5f) : 0;

  // SHADOW (SAMA)

  SDL_SetRenderDrawColor(renderer, 12, 20, 15, 150);
  SDL_Rect shadow{screenBody.x + 3, screenBody.y + 27, 22, 7};
  SDL_RenderFillRect(renderer, &shadow);

  // legs

  SDL_SetRenderDrawColor(renderer, 58, 43, 34, 255);

  SDL_Rect backLeg{screenBody.x + 5 + oppositeSwing, screenBody.y + 22 + bob, 6,
                   8}; // head
  //

  SDL_RenderFillRect(renderer, &backLeg);

  SDL_Rect frontLeg{screenBody.x + 14 + legSwing, screenBody.y + 22 + bob, 6,
                    8};

  SDL_RenderFillRect(renderer, &frontLeg);

  SDL_SetRenderDrawColor(renderer, 222, 183, 135, 255);
  SDL_Rect head{screenBody.x + 5, screenBody.y + bob, 14, 12};
  SDL_RenderFillRect(renderer, &head);

  // cloak

  SDL_SetRenderDrawColor(renderer, 75, 110, 75, 255);
  SDL_Rect cloak{screenBody.x + 2, screenBody.y + 12 + bob, 20, 16};
  SDL_RenderFillRect(renderer, &cloak);

  // swinging area

  SDL_SetRenderDrawColor(renderer, 63, 91, 61, 255);
  SDL_Rect leftArm{screenBody.x + 1, screenBody.y + 14 + bob + legSwing, 4, 10};

  SDL_RenderFillRect(renderer, &leftArm);

  SDL_Rect rightArm{screenBody.x + 19, screenBody.y + 14 + bob + oppositeSwing,
                    4, 10};
  SDL_RenderFillRect(renderer, &rightArm);

  // Boots
  SDL_SetRenderDrawColor(renderer, 75, 51, 37, 255);

  SDL_Rect backBoot{backLeg.x - 1, backLeg.y + 6, 8, 4};

  SDL_RenderFillRect(renderer, &backBoot);

  SDL_Rect frontBoot{frontLeg.x - 1, frontLeg.y + 6, 8, 4};

  SDL_RenderFillRect(renderer, &frontBoot);
}
void Player::reset() {
  x = 480.0f;
  y = 350.0f;
}

float Player::getX() const { return x; }
float Player::getY() const { return y; }

void Player::setPosition(float newX, float newY) {
  x = newX;
  y = newY;
  isMoving = false;
  walkTime = 0.0f;
}
