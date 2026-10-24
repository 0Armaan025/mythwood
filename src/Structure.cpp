

#include "../include/Structure.h"
#include <algorithm>
#include <utility>

Structure::Structure(StructureType type, SDL_Rect bounds, std::string name)
    : type(type), bounds(bounds), name(std::move(name)) {}

void Structure::render(SDL_Renderer *renderer, float cameraX,
                       float cameraY) const {

  SDL_Rect drawBounds = bounds;

  drawBounds.x -= static_cast<int>(cameraX);
  drawBounds.y -= static_cast<int>(cameraY);

  switch (type) {
  case StructureType::Chest: {
    SDL_SetRenderDrawColor(renderer, 125, 78, 43, 255);
    SDL_RenderFillRect(renderer, &drawBounds);

    SDL_SetRenderDrawColor(renderer, 214, 174, 86, 255);
    SDL_RenderDrawRect(renderer, &drawBounds);

    SDL_RenderDrawLine(renderer, drawBounds.x, drawBounds.y + drawBounds.h / 2,
                       drawBounds.x + drawBounds.w,
                       drawBounds.y + drawBounds.h / 2);

    if (used) {
      SDL_SetRenderDrawColor(renderer, 75, 55, 35, 255);

      SDL_RenderDrawLine(renderer, drawBounds.x + 3, drawBounds.y + 3,
                         drawBounds.x + drawBounds.w - 3,
                         drawBounds.y + drawBounds.h - 3);
    }

    break;
  }

  case StructureType::AncientShrine: {
    SDL_SetRenderDrawColor(renderer, 92, 105, 91, 255);
    SDL_RenderFillRect(renderer, &drawBounds);

    SDL_SetRenderDrawColor(renderer, 148, 171, 124, 255);
    SDL_RenderDrawRect(renderer, &drawBounds);

    SDL_Rect rune{drawBounds.x + drawBounds.w / 2 - 3,
                  drawBounds.y + drawBounds.h / 2 - 3, 6, 6};

    SDL_RenderFillRect(renderer, &rune);
    break;
  }

  case StructureType::Cabin: {
    SDL_SetRenderDrawColor(renderer, 105, 74, 51, 255);
    SDL_RenderFillRect(renderer, &drawBounds);

    SDL_SetRenderDrawColor(renderer, 65, 43, 34, 255);

    SDL_Rect roof{drawBounds.x - 5, drawBounds.y - 8, drawBounds.w + 10, 10};

    SDL_RenderFillRect(renderer, &roof);
    SDL_RenderDrawRect(renderer, &drawBounds);
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

    if (!inventory.addItem({"moonfire_sigil", "Moonfire Sigil [P]", 1, 1})) {
      return false;
    }

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
