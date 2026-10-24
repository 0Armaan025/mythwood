#include "../include/World.h"
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>

namespace {
constexpr int TILE_SIZE = 32;
constexpr int CHUNK_TILES = 16;
constexpr int CHUNK_SIZE = TILE_SIZE * CHUNK_TILES;

std::uint32_t terrainHash(int x, int y) {
  std::uint32_t h = static_cast<std::uint32_t>(x) * 374761393u +
                    static_cast<std::uint32_t>(y) * 668265263u;

  h = (h ^ (h >> 13)) * 1274126177u;
  return h ^ (h >> 16);
}
} // namespace

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
  enemies.clear();
  enemies.emplace_back(500.0f, 420.0f);
  player.reset();
  projectiles.clear();
  magicEquipped = false;
  previousP = false;
  shotCoolDown = 0.0f;
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
  const bool pDown = keys[SDL_SCANCODE_P];

  if (iDown && !previousI)
    inventoryOpen = !inventoryOpen;

  // OUR GUN STUFF

  if (pDown && !previousP) {
    if (magicEquipped) {
      magicEquipped = false;
      message = "Moonfire Sigil unequipped";
    } else if (inventory.countItem("moonfire_sigil") > 0) {
      magicEquipped = true;
      message = "Moonfire Sigil equipped! Left-click to cast.";
    } else {
      message = "you need to discover the Moonfire Sigil first.";
    }
  }

  previousP = pDown;

  if (!inventoryOpen) {
    for (auto &structure : structures) {
      if (structure.isNear(player.getBounds())) {
        nearbyStructure = &structure;
        break;
      }
    }
  }

  if (eDown && !previousE) {

    nearbyStructure = nullptr;

    for (auto &structure : structures) {
      if (structure.isNear(player.getBounds())) {
        nearbyStructure = &structure;
        break;
      }
    }

    if (eDown && !previousE) {
      if (nearbyStructure) {
        if (nearbyStructure->interact(inventory)) {
          message = "Found something at " + nearbyStructure->getName() + "!";
        } else if (nearbyStructure->isUsed()) {

          message = nearbyStructure->getName() + " has already been explored.";
        } else if (nearbyStructure->getBounds().w > 0) {
          message = nearbyStructure->getName() + " cannot be opened yet.";
        }
      }

      else {
        message = "Nothing nearby to interact with.";
      }
    }
  }

  previousE = eDown;
  previousI = iDown;

  if (!inventoryOpen) {
    std::vector<SDL_Rect> obstacles;
    obstacles.reserve(structures.size());

    for (const auto &structure : structures) {
      obstacles.push_back(structure.getBounds());
    }

    player.update(deltaTime, obstacles);
  }

  shotCoolDown = shotCoolDown - deltaTime;

  for (auto &enemy : enemies) {
    enemy.update(deltaTime);
  }

  for (auto &projectile : projectiles) {

    projectile.update(deltaTime);
  }

  for (auto &projectile : projectiles) {
    if (!projectile.isAlive())
      continue;

    const SDL_Rect projectileBounds = projectile.getBounds();

    for (auto &enemy : enemies) {
      if (!enemy.isAlive())
        continue;

      const SDL_Rect enemyBounds = enemy.getBounds();

      if (SDL_HasIntersection(&projectileBounds, &enemyBounds)) {
        enemy.takeDamage(10);
        projectile.destroy();
        break;
      }
    }

    for (const auto &structure : structures) {
      const SDL_Rect structureBounds = structure.getBounds();

      if (SDL_HasIntersection(&projectileBounds, &structureBounds)) {
        projectile.destroy();

        break;
      }
    }
  }

  int mouseX = 0;
  int mouseY = 0;
  const Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);

  if (!inventoryOpen && magicEquipped && (mouseButtons & SDL_BUTTON_LMASK) &&
      shotCoolDown <= 0.0f) {

    const SDL_Rect playerBounds = player.getBounds();
    const float startX = playerBounds.x + playerBounds.w / 2.0f;
    const float startY = playerBounds.y + playerBounds.h / 2.0f;

    projectiles.emplace_back(startX, startY,
                             static_cast<float>(mouseX) + cameraX,
                             static_cast<float>(mouseY) + cameraY);

    shotCoolDown = SHOT_INTERVAL;
  }

  projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
                                   [](const Projectile &projectile) {
                                     return !projectile.isAlive();
                                   }),
                    projectiles.end());

  // FOR INFINITE GEN SAKE

  const SDL_Rect playerBounds = player.getBounds();

  cameraX = playerBounds.x + playerBounds.w / 2.0f - 960.0f / 2.0f;
  cameraY = playerBounds.y + playerBounds.h / 2.0f - 640.0f / 2.0f;
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
  // SDL_SetRenderDrawColor(renderer, 42, 65, 44, 255);
  //
  // for (int i = 0; i < 30; ++i) {
  //
  //   const int worldX = (i * 137) % 2400;
  //   const int worldY = (i * 193) % 1800;
  //   SDL_Rect patch{worldX - static_cast<int>(cameraX),
  //                  worldY - static_cast<int>(cameraY), 3, 3};
  //   SDL_SetRenderDrawColor(renderer, 65, 115, 60, 255);
  //   SDL_RenderFillRect(renderer, &patch);
  // }

  // UPDATED GRASS PATCHES
  // terrain

  const int cameraWorldX = static_cast<int>(std::floor(cameraX));
  const int cameraWorldY = static_cast<int>(std::floor(cameraY));

  const int firstTileX =
      static_cast<int>(std::floor(static_cast<double>(cameraX) / TILE_SIZE));

  const int firstTileY =
      static_cast<int>(std::floor(static_cast<double>(cameraY) / TILE_SIZE));

  const int lastTileX =
      static_cast<int>(std::floor((cameraX + 960.0f) / TILE_SIZE));

  const int lastTileY =
      static_cast<int>(std::floor((cameraY + 640.0f) / TILE_SIZE));

  for (int ty = firstTileY; ty <= lastTileY; ++ty) {
    for (int tx = firstTileX; tx <= lastTileX; ++tx) {
      const std::uint32_t hash = terrainHash(tx, ty);

      SDL_Rect tile{tx * TILE_SIZE - cameraWorldX,
                    ty * TILE_SIZE - cameraWorldY, TILE_SIZE, TILE_SIZE};

      const Uint8 shade = static_cast<Uint8>(hash % 12);

      SDL_SetRenderDrawColor(renderer, 35 + shade, 59 + shade, 38 + shade / 2,
                             255);

      if (hash % 5 == 0) {
        const int dx = 4 + static_cast<int>((hash >> 8) % 21);
        const int dy = 4 + static_cast<int>((hash >> 16) % 21);

        SDL_Rect patch{tile.x + dx, tile.y + dy,
                       3 + static_cast<int>((hash >> 24) % 4), 2};

        SDL_SetRenderDrawColor(renderer, 58, 91, 48, 255);
        SDL_RenderFillRect(renderer, &patch);
      }

      // flowers coz why not soemtimeds

      if (hash % 47 == 0) {
        SDL_Rect flower{tile.x + 12, tile.y + 10, 3, 3};
        SDL_SetRenderDrawColor(renderer, 190, 174, 125, 255);
        SDL_RenderFillRect(renderer, &flower);
      }
    }
  }

  // Structures behind the player
  for (const auto &structure : structures)
    structure.render(renderer, cameraX, cameraY);

  player.render(renderer, cameraX, cameraY);

  // HAND AND TIP(GUN)

  if (magicEquipped) {
    int mouseX = 0;
    int mouseY = 0;
    SDL_GetMouseState(&mouseX, &mouseY);

    const SDL_Rect bounds = player.getBounds();

    const SDL_Rect playerBounds = player.getBounds();

    const float playerScreenX = playerBounds.x - cameraX;
    const float playerScreenY = playerBounds.y - cameraY;

    const float cx = playerScreenX + playerBounds.w / 2.0f;
    const float cy = playerScreenY + playerBounds.h / 2.0f;

    const float angle = std::atan2(static_cast<float>(mouseY) - cy,
                                   static_cast<float>(mouseX) - cx);

    const float handOffset = 8.0f;
    const float tipOffset = 20.0f;

    const float handX = cx + std::cos(angle) * handOffset;
    const float handY = cy + std::sin(angle) * handOffset;

    const float tipX = cx + std::cos(angle) * tipOffset;
    const float tipY = cy + std::sin(angle) * tipOffset;

    SDL_SetRenderDrawColor(renderer, 95, 55, 35, 255);

    SDL_RenderDrawLine(renderer, static_cast<int>(handX),
                       static_cast<int>(handY), static_cast<int>(tipX),
                       static_cast<int>(tipY));
    SDL_SetRenderDrawColor(renderer, 175, 115, 255, 255);

    // jin woo lol
    SDL_Rect crystal = {static_cast<int>(tipX) - 3, static_cast<int>(tipY) - 3,
                        7, 7};

    SDL_RenderFillRect(renderer, &crystal);
  }

  for (const auto &projectile : projectiles) {
    projectile.render(renderer, cameraX, cameraY);
  }

  for (const auto &enemy : enemies) {
    enemy.render(renderer, cameraX, cameraY);
  }

  // Top HUD
  SDL_SetRenderDrawColor(renderer, 20, 29, 25, 255);
  SDL_Rect bar{0, 0, 960, 64};
  SDL_RenderFillRect(renderer, &bar);

  SDL_SetRenderDrawColor(renderer, 105, 119, 82, 255);
  SDL_RenderDrawLine(renderer, 0, 63, 960, 63);

  drawText(renderer, "MYTHWOOD", 24, 17, {220, 204, 158, 255});
  // Interaction feedback
  SDL_SetRenderDrawColor(renderer, 20, 29, 25, 230);
  SDL_Rect messagePanel{20, 570, 500, 48};
  SDL_RenderFillRect(renderer, &messagePanel);

  drawText(renderer, "E: Interact   I: Inventory   P: Equip Sigil", 440, 22,
           {160, 174, 142, 255});
  drawText(renderer,
           magicEquipped ? "Moonfire: EQUIPPED" : "Moonfire: Unequipped", 24,
           42, {190, 199, 170, 255});
  if (nearbyStructure) {
    const std::string prompt = "E - Interact: " + nearbyStructure->getName();

    drawText(renderer, prompt, 32, 550, {190, 199, 170, 255});
  }

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
