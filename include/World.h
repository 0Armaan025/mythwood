#pragma once

#include "./Player.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>
#include <vector>

#include "../include/Inventory.h"
#include "../include/Structure.h"
class World {
public:
  ~World(); // ewww

  bool init();
  void start();
  void update(float deltaTime);
  void render(SDL_Renderer *renderer) const;
  bool shouldReturnToMenu() const; // yay at least this has been done :P

private:
  Player player;
  Inventory inventory;
  std::vector<Structure> structures;

  TTF_Font *font = nullptr;

  bool returnToMenu = false;
  bool inventoryOpen = false;
  bool previousE = false;
  bool previousI = false;

  Structure *nearbyStructure = nullptr;

  string message = "Explore the forest.";

  void drawText(SDL_Renderer *renderer, const std::string &text, int x, int y,
                SDL_Color color) const;
};
