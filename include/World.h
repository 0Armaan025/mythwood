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
#include <cstdint>
#include <unordered_set>

struct ResourceNode {
  enum class Type { Tree, Rock };

  int tileX;
  int tileY;
  Type type;
  SDL_Rect bounds;
};

class World {
public:
  ~World(); // ewww

  bool init();
  void start();
  void update(float deltaTime, SDL_Renderer *renderer);
  void render(SDL_Renderer *renderer) const;
  bool shouldReturnToMenu() const; // yay at least this has been done :P

  bool saveGame() const;
  bool loadGame();

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

  std::vector<ResourceNode> resourceNodes;
  std::unordered_set<std::uint64_t> harvestedResourceKeys;

  ResourceNode *nearbyResource = nullptr;

  // CHUNK STUFFY

  float cameraX = 0.0f;
  float cameraY = 0.0f;
};
