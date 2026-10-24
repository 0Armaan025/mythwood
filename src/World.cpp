#include "../include/World.h"
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>

#include <fstream>
#include <iomanip>
#include <limits>

#include <type_traits>

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

std::uint64_t resourcekey(int x, int y) {
  return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(x)) << 32) |
         static_cast<std::uint32_t>(y);
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

  resourceNodes.clear();
  harvestedResourceKeys.clear();
  nearbyResource = nullptr;
  nearbyStructure = nullptr;
  cameraX = 0.0f;
  cameraY = 0.0f;

  previousAttack = false;
  attackCooldown = 0.0f;

  attackAnimationTimer = 0.0f;

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

void World::update(float deltaTime, SDL_Renderer *renderer) {
  const Uint8 *keys = SDL_GetKeyboardState(nullptr);

  if (keys[SDL_SCANCODE_ESCAPE]) {
    if (!saveGame()) {
      message = "Warning: could not save the game!";
    }

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

  resourceNodes.clear();
  nearbyResource = nullptr;

  const int firstResourceTileX =
      static_cast<int>(std::floor(cameraX / TILE_SIZE)) - 2;

  const int firstResourceTileY =
      static_cast<int>(std::floor(cameraY / TILE_SIZE)) - 2;
  const int lastResourceTileX =
      static_cast<int>(std::floor((cameraX + 960.0f) / TILE_SIZE)) + 2;
  const int lastResourceTileY =
      static_cast<int>(std::floor((cameraY + 640.0f) / TILE_SIZE)) + 2;

  for (int ty = firstResourceTileY; ty <= lastResourceTileY; ++ty) {
    for (int tx = firstResourceTileX; tx <= lastResourceTileX; ++tx) {
      const std::uint32_t hash = terrainHash(tx, ty);

      ResourceNode::Type type;
      SDL_Rect bounds{};

      if (hash % 83 == 0) {
        type = ResourceNode::Type::Rock;
        bounds = {tx * TILE_SIZE + 9, ty * TILE_SIZE + 10, 15, 12};

      } else if (hash % 61 == 0) {
        type = ResourceNode::Type::Tree;
        bounds = {tx * TILE_SIZE + 6, ty * TILE_SIZE + 3, 20, 25};
      } else {
        continue;
      }
      if (harvestedResourceKeys.count(resourcekey(tx, ty)) != 0)
        continue;

      resourceNodes.push_back({tx, ty, type, bounds});
    }
  }

  if (!inventoryOpen) {
    for (auto &structure : structures) {
      if (structure.isNear(player.getBounds())) {
        nearbyStructure = &structure;
        break;
      }
    }
  }

  // if (eDown && !previousE) {
  //
  //   nearbyStructure = nullptr;
  //
  //   for (auto &structure : structures) {
  //     if (structure.isNear(player.getBounds())) {
  //       nearbyStructure = &structure;
  //       break;
  //     }
  //   }
  //
  //   if (eDown && !previousE) {
  //   if (nearbyStructure) {
  //     if (nearbyStructure->interact(inventory)) {
  //       message = "Found something at " + nearbyStructure->getName() + "!";
  //     } else if (nearbyStructure->isUsed()) {
  //
  //       message = nearbyStructure->getName() + " has already been
  //                                              explored.";
  //     } else if (nearbyStructure->getBounds().w > 0) {
  //       message = nearbyStructure->getName() + " cannot be opened yet.";
  //     }
  //   }
  //
  //   else {
  //     message = "Nothing nearby to interact with.";
  //   }
  // }
  // }
  //
  //

  nearbyStructure = nullptr;
  nearbyResource = nullptr;

  if (!inventoryOpen) {
    for (auto &structure : structures) {
      if (structure.isNear(player.getBounds())) {
        nearbyStructure = &structure;
        break;
      }
    }

    if (!nearbyStructure) {
      for (auto &node : resourceNodes) {
        if (harvestedResourceKeys.count(resourcekey(node.tileX, node.tileY)) !=
            0)
          continue;

        SDL_Rect range{node.bounds.x - 50, node.bounds.y - 50,
                       node.bounds.w + 100, node.bounds.h + 100};

        const SDL_Rect playerBounds = player.getBounds();
        if (SDL_HasIntersection(&playerBounds, &range)) {
          nearbyResource = &node;
          break;
        }
      }
    }

    if (eDown && !previousE && !inventoryOpen) {
      if (nearbyStructure) {
        if (nearbyStructure->interact(inventory)) {
          message = "Found something at " + nearbyStructure->getName() + "!";
        } else if (nearbyStructure->isUsed()) {

          message = nearbyStructure->getName() + "has already been explored.";
        } else if (nearbyStructure->getBounds().w > 0) {
          message = nearbyStructure->getName() + " cannot be opened yet.";
        }
      } else if (nearbyResource) {

        const std::string prompt =
            nearbyResource->type == ResourceNode::Type::Tree
                ? "E - Harvest tree"
                : "E - Mine rock";

        const auto key =
            resourcekey(nearbyResource->tileX, nearbyResource->tileY);

        const bool isTree = nearbyResource->type == ResourceNode::Type::Tree;

        const Item drop = isTree ? Item{"wood", "Wood", 1, 99}
                                 : Item{"stone", "Stone", 1, 99};

        if (inventory.addItem(drop)) {
          harvestedResourceKeys.insert(key);
          message = isTree ? "You gathered 1 wood." : "You collected 1 stone.";

          nearbyResource = nullptr;
        } else {
          message = "Inventory full! make room first...";
        }

      } else {
        message = "Nothing nearby to interact with.";
      }
    }
  }

  previousE = eDown;
  previousI = iDown;

  if (!inventoryOpen) {
    std::vector<SDL_Rect> obstacles;
    obstacles.reserve(structures.size() + resourceNodes.size());

    for (const auto &structure : structures) {
      obstacles.push_back(structure.getBounds());
    }
    for (const auto &node : resourceNodes) {
      if (harvestedResourceKeys.count(resourcekey(node.tileX, node.tileY)) ==
          0) {

        obstacles.push_back(node.bounds);
      }
    }

    player.update(deltaTime, obstacles);
  }

  shotCoolDown = shotCoolDown - deltaTime;

  attackCooldown = std::max(0.0f, attackCooldown - deltaTime);

  attackAnimationTimer = std::max(0.0f, attackAnimationTimer - deltaTime);

  const Uint8 *attackKeys = SDL_GetKeyboardState(nullptr);
  const bool attackDown = attackKeys[SDL_SCANCODE_SPACE];

  if (!inventoryOpen && attackDown && !previousAttack &&
      attackCooldown <= 0.0f) {
    const SDL_Rect playerBounds = player.getBounds();

    constexpr int attackWidth = 34;
    constexpr int attackHeight = 30;

    SDL_Rect attackBounds{};

    const float fx = player.getFacingX();
    const float fy = player.getFacingY();

    if (fx < 0.0f) {
      attackBounds = {playerBounds.x - attackWidth,
                      playerBounds.y + (playerBounds.h - attackHeight) / 2,
                      attackWidth, attackHeight};

    } else if (fx > 0.0f) {

      attackBounds = {playerBounds.x + (playerBounds.w - attackWidth) / 2,
                      playerBounds.y - attackHeight, attackWidth, attackHeight};

    }

    else {
      attackBounds = {playerBounds.x + (playerBounds.w - attackWidth) / 2,
                      playerBounds.y + playerBounds.h, attackWidth,
                      attackHeight};
    }
    for (auto &enemy : enemies) {
      if (!enemy.isAlive()) {
        continue;
      }

      SDL_Rect enemyBounds = enemy.getBounds();

      if (SDL_HasIntersection(&attackBounds, &enemyBounds)) {
        cout << "has intersection" << endl;
        enemy.takeDamage(MELEE_DAMAGE);
      }

      attackCooldown = ATTACK_COOLDOWN;
      attackAnimationTimer = ATTACK_ANIMATION_DURATION;
    }
  }

  previousAttack = attackDown;

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

      SDL_RenderFillRect(renderer, &tile);

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

      for (const auto &node : resourceNodes) {
        if (harvestedResourceKeys.count(resourcekey(node.tileX, node.tileY)) !=
            0)
          continue;

        SDL_Rect drawBounds = node.bounds;
        drawBounds.x -= static_cast<int>(cameraX);
        drawBounds.y -= static_cast<int>(cameraY);

        if (node.type == ResourceNode::Type::Tree) {
          SDL_Rect trunk{drawBounds.x + drawBounds.w / 2 - 3,
                         drawBounds.y + drawBounds.h / 2, 6, drawBounds.h / 2};

          SDL_SetRenderDrawColor(renderer, 91, 62, 39, 255);
          SDL_RenderFillRect(renderer, &trunk);

          SDL_Rect leaves{drawBounds.x, drawBounds.y, drawBounds.w,
                          drawBounds.h * 2 / 3};

          SDL_SetRenderDrawColor(renderer, 39, 91, 48, 255);
          SDL_RenderFillRect(renderer, &leaves);

          SDL_SetRenderDrawColor(renderer, 60, 116, 63, 255);
          SDL_RenderDrawRect(renderer, &leaves);
        } else {
          SDL_SetRenderDrawColor(renderer, 92, 96, 88, 255);
          SDL_RenderFillRect(renderer, &drawBounds);
          // uhhh bored

          SDL_SetRenderDrawColor(renderer, 145, 149, 136, 255);
          SDL_RenderDrawLine(renderer, drawBounds.x + 2, drawBounds.y + 3,
                             drawBounds.x + 8, drawBounds.y + 2);
        }
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

bool World::saveGame() const {
  const std::string tempPath = "mythwood_save.tmp";
  const std::string savePath = "mythwood_save.dat";

  std::ofstream out(tempPath, std::ios::trunc);

  if (!out)
    return false;

  out << "MYTHWOOD_SAVE 1\n";
  out << std::setprecision(std::numeric_limits<float>::max_digits10);

  out << player.getX() << ' ' << player.getY() << '\n';

  const auto &items = inventory.getItems();
  out << items.size() << '\n';

  for (const auto &item : items) {
    out << std::quoted(item.id) << ' ' << std::quoted(item.name) << ' '
        << item.quantity << ' ' << item.maxStack << '\n';
  }

  out << structures.size() << '\n';

  for (const auto &structure : structures) {
    out << structure.isUsed() << '\n';
  }
  out << harvestedResourceKeys.size() << '\n';
  for (const auto key : harvestedResourceKeys) {
    out << key << '\n';
  }

  out.close();

  if (!out) {
    return false;
  }

  if (std::rename(tempPath.c_str(), savePath.c_str()) != 0) {
    return false;
  }

  return true;
}

bool World::loadGame() {
  std::ifstream in("mythwood_save.dat");

  if (!in)
    return false;

  std::string magic;
  int version = 0;

  if (!(in >> magic >> version) || magic != "MYTHWOOD_SAVE" || version != 1) {
    return false;
  }

  float loadedX = 0.0f;
  float loadedY = 0.0f;

  if (!(in >> loadedX >> loadedY) || !std::isfinite(loadedX) ||
      !std::isfinite(loadedY)) {
    return false;
  }

  // million for now ig
  constexpr float MAX_WORLD_COORDINATESS = 1000000.0f;

  if (std::abs(loadedX) > MAX_WORLD_COORDINATESS ||
      std::abs(loadedY) > MAX_WORLD_COORDINATESS) {
    return false;
  }

  std::size_t itemCount = 0;
  if (!(in >> itemCount) || itemCount > 20) {
    return false;
  }

  std::vector<Item> loadedItems;
  // loadedItems.reserve(itemCountt)
  loadedItems.reserve(itemCount);

  for (std::size_t i = 0; i < itemCount; ++i) {
    Item item;

    if (!(in >> std::quoted(item.id) >> std::quoted(item.name) >>
          item.quantity >> item.maxStack))
      return false;

    if (item.id.empty() || item.name.empty() || item.quantity <= 0 ||
        item.maxStack <= 0 || item.quantity > item.maxStack)
      return false;

    loadedItems.push_back(std::move(item));
  }

  std::size_t structureCount = 0;
  if (!(in >> structureCount) || structureCount != 3)
    return false;

  std::vector<bool> loadedStructureStates;
  loadedStructureStates.reserve(structureCount);

  for (std::size_t i = 0; i < structureCount; ++i) {
    int used = 0;

    if (!(in >> used) || (used != 0 && used != 1)) {
      return false;
    }

    loadedStructureStates.push_back(used == 1);
  }

  std::size_t harvestedCount = 0;

  if (!(in >> harvestedCount) || harvestedCount > 1000000)
    return false;

  std::unordered_set<std::uint64_t> loadedHarvestedKeys;

  for (std::size_t i = 0; i < harvestedCount; ++i) {
    std::uint64_t key = 0;
    if (!(in >> key)) {
      return false;
    }

    loadedHarvestedKeys.insert(key);
  }

  Inventory validatedInventory;

  if (!validatedInventory.setItems(loadedItems))
    return false;

  // finally start

  start();

  player.setPosition(loadedX, loadedY);
  inventory.setItems(loadedItems);
  for (std::size_t i = 0; i < structures.size(); ++i) {
    structures[i].setUsed(loadedStructureStates[i]);
  }

  harvestedResourceKeys = std::move(loadedHarvestedKeys);

  const SDL_Rect bounds = player.getBounds();
  cameraX = bounds.x + bounds.w / 2.0f - 480.0f;
  cameraY = bounds.y + bounds.h / 2.0f - 320.0f;

  message = "Save loaded successfully.";
  returnToMenu = false;
  return true;
}
