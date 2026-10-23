#include "../include/World.h"
#include <iostream>

World::~World() {
  if (font)
    TTF_CloseFont(font);
}

bool World::init() {
  font = TTF_OpenFont("/usr/share/fonts/TTF/DejaVuSerif.ttf", 18);

  if (!font) {
    cout << "error: " << TTF_GetError() << endl;
    return false;
  }

  start();
  return true;
}

void World::start() {
  player.reset();
  inventory = Inventory{};
  structures.clear();

  structures.emplace_back(StructureType::Chest, SDL_Rect{390, 270, 34, 28},
                          "Old Chest");

  structures.emplace_back(StructureType::AncientShrine,
                          SDL_Rect{650, 180, 48, 52}, "Ancient Shrine");

  structures.emplace_back(StructureType::Cabin, SDL_Rect{140, 160, 100, 80},
                          "Forest Cabin");

  inventoryOpen = false;
  returnToMenu = false;
  previousE = false;
  previousI = false;
  message = "Explore the forest.";
}

void World::update(float deltaTime) {
  const Uint8 *keys = SDL_GetKeyboardState(nullptr);

  if (keys[SDL_SCANCODE_ESCAPE]) {
    returnToMenu = true;
    return;
  }

  const bool eDown = keys[SDL_SCANCODE_E];
  const bool iDown = keys[SDL_SCANCODE_I];

  if (iDown && !previousI)
    inventoryOpen = !inventoryOpen;

  if (eDown && !previousE) {
    std::cout << "E pressed in World::update()" << endl;
    bool found = false;

    for (auto &structure : structures) {
      if (!structure.isNear(player.getBounds()))
        continue;

      found = true;

      if (structure.interact(inventory)) {
        message = "Found something at " + structure.getName() + "!";
      } else if (structure.isUsed()) {
        message = structure.getName() + " has already been explored.";
      } else {
        message = structure.getName() + " cannot be opened yet.";
      }

      break;
    }

    if (!found)
      message = "Nothing nearby to interact with.";
  }

  cout << "Message: " << message << endl;

  previousE = eDown;
  previousI = iDown;

  if (!inventoryOpen)
    player.update(deltaTime);
}

void World::drawText(SDL_Renderer *renderer, const string &text, int x, int y,
                     SDL_Color color) const {
  SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);

  if (!surface) {
    std::cerr << "TTF_RenderUTF8_Blended failed: " << TTF_GetError()
              << std::endl;
    return;
  }
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

  if (!texture) {
    std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError()
              << std::endl;
  } else {
    SDL_Rect dest{x, y, surface->w, surface->h};

    if (SDL_RenderCopy(renderer, texture, nullptr, &dest) != 0)
      std::cerr << "SDL_RenderCopy failed: " << SDL_GetError() << std::endl;

    SDL_DestroyTexture(texture);
  }
  SDL_FreeSurface(surface);
}

void World::render(SDL_Renderer *renderer) const {
  SDL_SetRenderDrawColor(renderer, 35, 55, 39, 255);
  SDL_RenderClear(renderer);

  // Grass patches
  SDL_SetRenderDrawColor(renderer, 42, 65, 44, 255);

  for (int i = 0; i < 30; ++i) {
    SDL_Rect patch{(i * 137 + 29) % 950, 85 + (i * 83) % 530, 22, 12};
    SDL_RenderFillRect(renderer, &patch);
  }

  // Structures behind the player
  for (const auto &structure : structures)
    structure.render(renderer);

  player.render(renderer);

  // Top HUD
  SDL_SetRenderDrawColor(renderer, 20, 29, 25, 255);
  SDL_Rect bar{0, 0, 960, 64};
  SDL_RenderFillRect(renderer, &bar);

  SDL_SetRenderDrawColor(renderer, 105, 119, 82, 255);
  SDL_RenderDrawLine(renderer, 0, 63, 960, 63);

  std::cout << "Rendering message: " << message << std::endl;
  drawText(renderer, "MYTHWOOD", 24, 17, {220, 204, 158, 255});

  drawText(renderer, "E: Interact   I: Inventory", 650, 22,
           {160, 174, 142, 255});

  // Interaction feedback
  SDL_SetRenderDrawColor(renderer, 20, 29, 25, 230);
  SDL_Rect messagePanel{20, 570, 500, 48};
  SDL_RenderFillRect(renderer, &messagePanel);

  drawText(renderer, message, 32, 584, {220, 204, 158, 255});

  // Inventory panel
  if (inventoryOpen) {
    SDL_SetRenderDrawColor(renderer, 20, 29, 25, 245);
    SDL_Rect panel{610, 90, 320, 450};
    SDL_RenderFillRect(renderer, &panel);

    SDL_SetRenderDrawColor(renderer, 105, 119, 82, 255);
    SDL_RenderDrawRect(renderer, &panel);

    drawText(renderer, "INVENTORY", 635, 110, {220, 204, 158, 255});

    int y = 150;

    for (const auto &item : inventory.getItems()) {
      drawText(renderer, item.name + " x" + to_string(item.quantity), 635, y,
               {190, 199, 170, 255});

      y += 30;
    }

    if (inventory.getItems().empty())
      drawText(renderer, "Your bag is empty.", 635, 155, {160, 174, 142, 255});

    drawText(renderer, "Press I to close", 635, 500, {160, 174, 142, 255});
  }

  SDL_RenderPresent(renderer);
}

bool World::shouldReturnToMenu() const { return returnToMenu; }
