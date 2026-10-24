#pragma once
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
using namespace std;

class Player {
public:
  void
  update(float deltaTime,
         const vector<SDL_Rect> &
             obstacles); // OH THIS BRINGS BACK NOSTALGIA FROM WHEN I USED UNITY
  void render(SDL_Renderer *renderer, float cameraX = 0.0f,
              float cameraY = 0.0f) const;
  void reset();

  SDL_Rect getBounds() const;

  float getX() const;
  float getY() const;
  void setPosition(float newX, float newY);

private:
  float x = 480.0f;
  float y = 350.0f;
  float speed = 220.0f;
  float walkTime = 0.0f;
  bool isMoving = false;
};
