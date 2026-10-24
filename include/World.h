#pragma once

#include "./Player.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>
#include <vector>

#include "../include/Enemy.h"
#include "../include/Inventory.h"
#include "../include/Projectile.h"
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
  std::vector<Enemy> enemies;

  bool returnToMenu = false;
  bool inventoryOpen = false;
  bool previousE = false;
  bool previousI = false;

  Structure *nearbyStructure = nullptr;

  string message = "Explore the forest.";

  void drawText(SDL_Renderer *renderer, const std::string &text, int x, int y,
                SDL_Color color) const;

  std::vector<Projectile> projectiles;

  bool previousP = false;
  bool magicEquipped = false;

  float shotCoolDown = 0.0f;
  static constexpr float SHOT_INTERVAL = 0.25f;

  // CHUNK STUFFY

  float cameraX = 0.0f;
  float cameraY = 0.0f;
};
