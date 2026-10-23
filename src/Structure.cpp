

#include "../include/Structure.h"
#include <algorithm>
#include <utility>

Structure::Structure(StructureType type, SDL_Rect bounds, std::string name)
    : type(type), bounds(bounds), name(std::move(name)) {}

void Structure::render(SDL_Renderer *renderer) const {
  switch (type) {
  case StructureType::Chest: {
    SDL_SetRenderDrawColor(renderer, 125, 78, 43, 255);
    SDL_RenderFillRect(renderer, &bounds);

    SDL_SetRenderDrawColor(renderer, 214, 174, 86, 255);
    SDL_RenderDrawRect(renderer, &bounds);

    SDL_RenderDrawLine(renderer, bounds.x, bounds.y + bounds.h / 2,
                       bounds.x + bounds.w, bounds.y + bounds.h / 2);

    if (used) {
      SDL_SetRenderDrawColor(renderer, 75, 55, 35, 255);

      SDL_RenderDrawLine(renderer, bounds.x + 3, bounds.y + 3,
                         bounds.x + bounds.w - 3, bounds.y + bounds.h - 3);
    }
    break;
  }

  case StructureType::AncientShrine: {
    SDL_SetRenderDrawColor(renderer, 92, 105, 91, 255);
    SDL_RenderFillRect(renderer, &bounds);

    SDL_SetRenderDrawColor(renderer, 148, 171, 124, 255);
    SDL_RenderDrawRect(renderer, &bounds);

    SDL_Rect rune{bounds.x + bounds.w / 2 - 3, bounds.y + bounds.h / 2 - 3, 6,
                  6};

    SDL_RenderFillRect(renderer, &rune);
    break;
  }

  case StructureType::Cabin: {
    SDL_SetRenderDrawColor(renderer, 105, 74, 51, 255);
    SDL_RenderFillRect(renderer, &bounds);

    SDL_SetRenderDrawColor(renderer, 65, 43, 34, 255);

    SDL_Rect roof{bounds.x - 5, bounds.y - 8, bounds.w + 10, 10};

    SDL_RenderFillRect(renderer, &roof);
    SDL_RenderDrawRect(renderer, &bounds);
    break;
  }
  }
}

SDL_Rect Structure::getBounds() const { return bounds; }

bool Structure::isNear(SDL_Rect playerBounds) const {
  SDL_Rect range{bounds.x - 50, bounds.y - 50, bounds.w + 100, bounds.h + 100};

  return SDL_HasIntersection(&playerBounds, &range);
}

bool Structure::interact(Inventory &inventory) {
  if (used)
    return false;

  switch (type) {
  case StructureType::Chest: {
    if (!inventory.addItem({"moonleaf", "Moonleaf", 3, 20})) {
      return false;
    }

    used = true;
    return true;
  }

  case StructureType::AncientShrine: {
    if (!inventory.addItem({"ancient_rune", "Ancient Rune", 1, 10})) {
      return false;
    }

    used = true;
    return true;
  }

  case StructureType::Cabin: {
    return false;
  }
  }

  return false;
}

const std::string &Structure::getName() const { return name; }

bool Structure::isUsed() const { return used; }
