#pragma once

#include "../include/Inventory.h"
#include <SDL2/SDL.h>
#include <string>

enum class StructureType { Chest, AncientShrine, Cabin };

class Structure {
public:
  Structure(StructureType type, SDL_Rect bounds, string name);

  void render(SDL_Renderer *renderer, float cameraX = 0.0f,
              float cameraY = 0.0f) const;

  bool isNear(SDL_Rect playerBounds) const;

  bool interact(Inventory &inventory);

  const std::string &getName() const;

  bool isUsed() const;

  SDL_Rect getBounds() const;
  void setUsed(bool value);

private:
  StructureType type;
  SDL_Rect bounds;
  string name;
  bool used = false;
};
