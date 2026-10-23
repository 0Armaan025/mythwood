

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

bool Structure::isNear(SDL_Rect playerBounds) const {
  const int px = playerBounds.x + playerBounds.w / 2;
  const int py = playerBounds.y + playerBounds.h / 2;

  const int sx = bounds.x + bounds.w / 2;
  const int sy = bounds.y + bounds.h / 2;

  const int dx = px - sx;
  const int dy = py - sy;

  constexpr int INTERACTION_RADIUS = 60;

  return dx * dx + dy * dy <= INTERACTION_RADIUS * INTERACTION_RADIUS;
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
