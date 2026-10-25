#include "../include/World.h"
#include <SDL2/SDL_events.h>
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

  const SDL_Point slimeSpawns[] = {
      {500, 420}, {250, 350}, {350, 500}, {420, 150}, {560, 280},
      {600, 520}, {720, 350}, {800, 450}, {900, 250}, {950, 600},
      {150, 400}, {200, 550}, {700, 650}, {1000, 400}};

  for (const SDL_Point &spawn : slimeSpawns) {

    enemies.emplace_back(static_cast<float>(spawn.x),
                         static_cast<float>(spawn.y));
  }

  player.reset();
  projectiles.clear();
  magicEquipped = false;
  previousP = false;
  rockets.clear();
  hasRocketLauncher = false;
  launcherEquipped = false;
  rocketAmmo = 0;

  previousBuyLauncher = false;
  previousLauncherToggle = false;
  rocketCooldown = 0.0f;
  shotCoolDown = 0.0f;
  inventory = Inventory{};
  structures.clear();

  previousBuyPotion = false;
  previousUsePotion = false;

  coins = 0;
  shopOpen = false;
  previousShopSellWood = false;
  previousShopSellStone = false;
  resourceNodes.clear();
  harvestedResourceKeys.clear();
  nearbyResource = nullptr;
  nearbyStructure = nullptr;
  cameraX = 0.0f;
  cameraY = 0.0f;

  previousAttack = false;
  attackCooldown = 0.0f;

  attackAnimationTimer = 0.0f;

  playerHealth = MAX_PLAYER_HEALTH;
  playerDamageCoolDown = 0.0f;
  playerDead = false;
  respawnTimer = 0.0f;

  structures.emplace_back(StructureType::Chest, SDL_Rect{390, 270, 34, 28},
                          "Old Chest");

  structures.emplace_back(StructureType::AncientShrine,
                          SDL_Rect{650, 180, 48, 52}, "Ancient Shrine");

  structures.emplace_back(StructureType::Cabin, SDL_Rect{140, 160, 100, 80},
                          "Forest Cabin");

  structures.emplace_back(StructureType::Merchant, SDL_Rect{300, 160, 54, 58},
                          "Forest Merchant");

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

  const bool sellWoodDown = keys[SDL_SCANCODE_1];
  const bool sellStoneDown = keys[SDL_SCANCODE_2];
  const bool buyPotionDown = keys[SDL_SCANCODE_3];
  const bool buyLauncherDown = keys[SDL_SCANCODE_4];
  const bool launcherToggleDown = keys[SDL_SCANCODE_R];

  const bool usePotionDown = keys[SDL_SCANCODE_H];

  if (shopOpen) {
    if (eDown && !previousE) {
      shopOpen = false;
      previousE = true;
      message = "Come back anytime.";
    }

    if (sellWoodDown && !previousShopSellWood) {
      const int amount = inventory.countItem("wood");

      if (amount > 0 && inventory.removeItem("wood", amount)) {
        coins += amount * WOOD_SELL_PRICE;
        message = "Sold " + std::to_string(amount) + " wood for " +
                  std::to_string(amount * WOOD_SELL_PRICE) + " coins.";
      } else {
        message = "You don't have any wood to sell.";
      }
    }

    if (sellStoneDown && !previousShopSellStone) {
      const int amount = inventory.countItem("stone");

      if (amount > 0 && inventory.removeItem("stone", amount)) {
        coins += amount * STONE_SELL_PRICE;
        message = "Sold " + std::to_string(amount) + " stone for " +
                  std::to_string(amount * STONE_SELL_PRICE) + " coins.";

      } else {
        message = "You don't have any stone to sell.";
      }
    }

    if (buyPotionDown && !previousBuyPotion) {
      if (coins < POTION_PRICE) {
        message = "Not enough coins! you need 15.";

      }

      else if (inventory.addItem(
                   Item{"health_potion", "Health Potion", 1, 99})) {
        coins = coins - POTION_PRICE;

        message = "Bought a Health Potion1!";
      } else {
        message = "Inventory full!";
      }
    }
  }

  if (buyLauncherDown && !previousBuyLauncher) {
    if (hasRocketLauncher) {
      message = "You already own the Chaos Launcher!";
    } else if (coins < ROCKET_LAUNCHER_PRICE) {
      message = "You need 75 coins for the Chaos Launcher.";
    } else {
      coins -= ROCKET_LAUNCHER_PRICE;

      hasRocketLauncher = true;
      launcherEquipped = true;
      rocketAmmo = STARTING_ROCKET_AMMO;

      message = "CHAOS LAUNCHER acquired! Press R to equip!";
    }
  }

  if (!shopOpen && iDown && !previousI) {
    inventoryOpen = !inventoryOpen;
  }

  if (!shopOpen && usePotionDown && !previousUsePotion) {
    if (playerHealth >= MAX_PLAYER_HEALTH) {
      message = "Your health is already full.";

    } else if (inventory.removeItem("health_potion", 1)) {
      playerHealth =
          std::min(MAX_PLAYER_HEALTH, playerHealth + POTION_HEAL_AMOUNT);

      message = "Health restored!";
    } else {
      message = "You don't have a Health Potion.";
    }
  }

  previousBuyPotion = buyPotionDown;
  previousUsePotion = usePotionDown;

  previousShopSellWood = sellWoodDown;
  previousShopSellStone = sellStoneDown;

  if (!shopOpen && !inventoryOpen && launcherToggleDown &&
      !previousLauncherToggle) {
    if (!hasRocketLauncher) {
      message = "Find a merchant to buy the Chaos Launcher.";
    }

    else {
      launcherEquipped = !launcherEquipped;

      message = launcherEquipped
                    ? "CHAOS Launcher Equipped!"
                    : "Chaos Launcher holstered. Use P for Moonfire.";
    }
  }

  previousBuyLauncher = buyLauncherDown;
  previousLauncherToggle = launcherToggleDown;

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

  if (!inventoryOpen && !shopOpen) {
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

  if (!inventoryOpen && !shopOpen) {
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
        if (nearbyStructure->getType() == StructureType::Merchant) {
          shopOpen = true;
          inventoryOpen = false;
          message = "Welcome! Sell your resources with 1 or 2.";
        } else if (nearbyStructure->interact(inventory)) {
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

  if (!inventoryOpen && !shopOpen) {
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

  rocketCooldown = std::max(0.0f, rocketCooldown - deltaTime);

  attackCooldown = std::max(0.0f, attackCooldown - deltaTime);

  attackAnimationTimer = std::max(0.0f, attackAnimationTimer - deltaTime);

  const Uint8 *attackKeys = SDL_GetKeyboardState(nullptr);
  const bool attackDown = attackKeys[SDL_SCANCODE_SPACE];

  if (!inventoryOpen && !shopOpen && attackDown && !previousAttack &&
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

  const SDL_Rect boundsOfPlayer = player.getBounds();

  playerDamageCoolDown = std::max(0.0f, playerDamageCoolDown - deltaTime);

  if (playerDead) {
    respawnTimer = respawnTimer - deltaTime;

    if (respawnTimer <= 0.0f) {
      player.reset();
      playerHealth = MAX_PLAYER_HEALTH;
      playerDamageCoolDown = 0.0f;
      playerDead = false;
      message = " You recovered and returned to the forest. ";
    }
  } else {

    const SDL_Rect playerBounds = player.getBounds();

    for (auto &enemy : enemies) {

      if (!enemy.isAlive() || playerDamageCoolDown > 0.0f) {
        continue;
      }

      const SDL_Rect enemyBounds = enemy.getBounds();

      if (SDL_HasIntersection(&playerBounds, &enemyBounds)) {
        playerHealth = std::max(0, playerHealth - 10);
        playerDamageCoolDown = PLAYER_DAMAGE_COOLDOWN;

        message = "Slime hit u heehehe, noob";

        if (playerHealth <= 0) {
          playerDead = true;
          respawnTimer = RESPAWN_DELAY;
          message = "you were defeated...";
        }
      }
    }
  }

  for (auto &enemy : enemies) {
    enemy.update(deltaTime, boundsOfPlayer);
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

    for (auto &rocket : rockets) {
      rocket.update(deltaTime);

      if (rocket.isFlying()) {
        const SDL_Rect rocketBounds = rocket.getBounds();

        for (const auto &enemy : enemies) {
          if (!enemy.isAlive()) {
            continue;
          }

          const SDL_Rect enemyBounds = enemy.getBounds();

          if (SDL_HasIntersection(&rocketBounds, &enemyBounds)) {
            rocket.beginExplosion();
            break;
          }
        }
        if (rocket.isFlying()) {
          for (const auto &structure : structures) {

            const SDL_Rect structureBounds = structure.getBounds();

            if (SDL_HasIntersection(&rocketBounds, &structureBounds)) {
              // im tireeeeeeedddddddddd audashfalsdjfdlasjl

              rocket.beginExplosion();
              break;
            }
          }
        }
      }

      if (!rocket.consumeBlast()) {
        continue;
      }

      const float blastX = rocket.getCenterX();
      const float blastY = rocket.getCenterY();

      for (auto &enemy : enemies) {
        if (!enemy.isAlive()) {
          continue;
        }

        const SDL_Rect bounds = enemy.getBounds();

        const float enemyX = bounds.x + bounds.w / 2.0f;
        const float enemyY = bounds.y + bounds.h / 2.0f;

        const float dx = enemyX - blastX;
        const float dy = enemyY - blastY;
        const float distance = std::sqrt(dx * dx + dy * dy);

        if (distance > Rocket::BLAST_RADIUS) {
          continue;
        }

        enemy.takeDamage(Rocket::BLAST_DAMAGE);

        const float strength =
            Rocket::KNOCKBACK_FORCE * (1.0f - distance / Rocket::BLAST_RADIUS);

        if (strength > 0.0f) {
          enemy.applyKnockback(blastX, blastY, strength);
        }
      }
    }

    rockets.erase(
        std::remove_if(rockets.begin(), rockets.end(),
                       [](const Rocket &rocket) { return !rocket.isAlive(); }),
        rockets.end());

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

  if (!inventoryOpen && !shopOpen && magicEquipped &&
      (mouseButtons & SDL_BUTTON_LMASK) && shotCoolDown <= 0.0f) {

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

  if (!inventoryOpen && !shopOpen && launcherEquipped &&
      (mouseButtons && SDL_BUTTON_LMASK) && rocketCooldown <= 0.0f) {
  }

  if (rocketAmmo <= 0) {
    launcherEquipped = false;
    message = "Out of rockets! Launcher holstered.";
  } else {
    const SDL_Rect playerBounds = player.getBounds();

    const float startX = playerBounds.x + playerBounds.w / 2.0f;
    const float startY = playerBounds.y + playerBounds.h / 2.0f;

    rockets.emplace_back(startX, startY, static_cast<float>(mouseX) + cameraX,
                         static_cast<float>(mouseY) + cameraY);

    --rocketAmmo;
    rocketCooldown = ROCKET_FIRE_INTERVAL;
  }

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

  if (launcherEquipped) {
    int mouseX = 0;
    int mouseY = 0;
    SDL_GetMouseState(&mouseX, &mouseY);

    const SDL_Rect playerBounds = player.getBounds();

    const float cx = playerBounds.x - cameraX + playerBounds.w / 2.0f;
    const float cy = playerBounds.y - cameraY + playerBounds.h / 2.0f;

    const float angle = std::atan2(static_cast<float>(mouseY) - cy,
                                   static_cast<float>(mouseX) - cx);

    const float handX = cx + std::cos(angle) * 8.0f;
    const float handY = cy + std::sin(angle) * 8.0f;
    const float tipX = cx + std::cos(angle) * 23.0f;
    const float tipY = cy + std::sin(angle) * 23.0f;
    // barrele
    //

    const float perpX = -std::sin(angle) * 3.0f;
    const float perpY = std::cos(angle) * 3.0f;

    SDL_SetRenderDrawColor(renderer, 65, 43, 35, 255);

    SDL_RenderDrawLine(renderer, static_cast<int>(handX + perpX),
                       static_cast<int>(handY + perpY),
                       static_cast<int>(tipX + perpX),
                       static_cast<int>(tipY + perpY));

    SDL_RenderDrawLine(renderer, static_cast<int>(handX - perpX),
                       static_cast<int>(handY - perpY),
                       static_cast<int>(tipX - perpX),
                       static_cast<int>(tipY - perpY));

    SDL_SetRenderDrawColor(renderer, 255, 125, 35, 255);

    SDL_RenderDrawLine(renderer, static_cast<int>(handX),
                       static_cast<int>(handY), static_cast<int>(tipX),
                       static_cast<int>(tipY));

    SDL_Rect muzzle{static_cast<int>(tipX) - 3, static_cast<int>(tipY) - 3, 6,
                    6};

    SDL_SetRenderDrawColor(renderer, 55, 220, 115, 255);
    // uh im boreeed

    SDL_RenderFillRect(renderer, &muzzle);
  }

  for (const auto &projectile : projectiles) {
    projectile.render(renderer, cameraX, cameraY);
  }

  for (const auto &enemy : enemies) {
    enemy.render(renderer, cameraX, cameraY);
  }

  for (const auto &rocket : rockets) {
    rocket.render(renderer, cameraX, cameraY);
  }

  // Top HUD
  SDL_SetRenderDrawColor(renderer, 20, 29, 25, 255);
  SDL_Rect bar{0, 0, 960, 64};
  SDL_RenderFillRect(renderer, &bar);

  SDL_SetRenderDrawColor(renderer, 105, 119, 82, 255);
  SDL_RenderDrawLine(renderer, 0, 63, 960, 63);

  drawText(renderer, "MYTHWOOD", 24, 17, {220, 204, 158, 255});
  // Interaction feedback

  constexpr int healthBarX = 180;
  constexpr int healthBarY = 20;
  constexpr int healthBarWidth = 160;
  constexpr int healthBarHeight = 16;

  SDL_SetRenderDrawColor(renderer, 55, 35, 35, 255);

  SDL_Rect healthBackground{healthBarX, healthBarY, healthBarWidth,
                            healthBarHeight};
  SDL_RenderFillRect(renderer, &healthBackground);
  const float healthRatio =
      static_cast<float>(playerHealth) / MAX_PLAYER_HEALTH;
  SDL_SetRenderDrawColor(renderer, 190, 65, 70, 255);
  SDL_Rect healthForeground{
      healthBarX + 2, healthBarY + 2,
      static_cast<int>((healthBarWidth - 4) * healthRatio),
      healthBarHeight - 4};

  SDL_RenderFillRect(renderer, &healthForeground);

  drawText(renderer,
           "HP " + std::to_string(playerHealth) + "/" +
               std::to_string(MAX_PLAYER_HEALTH),
           healthBarX + healthBarWidth + 10, healthBarY - 2,
           {220, 204, 158, 255});

  SDL_SetRenderDrawColor(renderer, 20, 29, 25, 230);
  SDL_Rect messagePanel{20, 570, 500, 48};
  SDL_RenderFillRect(renderer, &messagePanel);

  drawText(renderer, "E: Interact   I: Inventory   P: Sigil   R: Launcher", 440,
           22, {160, 174, 142, 255});
  drawText(renderer,
           magicEquipped ? "Moonfire: EQUIPPED" : "Moonfire: Unequipped", 24,
           42, {190, 199, 170, 255});

  const std::string launcherStatus =
      !hasRocketLauncher
          ? "Launcher: not owned"
          : "Launcher: " + std::to_string(rocketAmmo) +
                (launcherEquipped ? " [EQUIPPED] " : " [R to equip] ");

  drawText(renderer, launcherStatus, 440, 42, {255, 170, 95, 255});

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
  if (shopOpen) {
    SDL_SetRenderDrawColor(renderer, 20, 29, 25, 245);
    SDL_Rect panel{260, 150, 440, 370};
    SDL_RenderFillRect(renderer, &panel);

    SDL_SetRenderDrawColor(renderer, 105, 119, 82, 255);
    SDL_RenderDrawRect(renderer, &panel);

    drawText(renderer, "FOREST MERCHANT", 290, 175, {220, 204, 158, 255});
    drawText(renderer, "Coins: " + std::to_string(coins), 290, 215,
             {240, 204, 158, 95});

    drawText(renderer, "1 - Sell all wood (3 coins each)", 290, 265,
             {190, 199, 170, 255});

    drawText(renderer, "2 - Sell all stone (2 coins each)", 290, 305,
             {190, 199, 170, 255});

    drawText(renderer, "3 - Buy Health Potion (15 coins each)", 290, 325,
             {190, 199, 170, 255});

    drawText(renderer,
             hasRocketLauncher ? "4 - Chaos Launcher (OWNED) "
                               : "4 - Buy Cahos Launcher (75 coinss)",
             290, 350, {255, 150, 65, 255});

    drawText(renderer, "H - Use a potion", 290, 390, {190, 199, 170, 255});
    drawText(renderer, "E - Leave shop", 290, 430, {160, 174, 142, 255});
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

  out << "MYTHWOOD_SAVE 2\n";
  out << std::setprecision(std::numeric_limits<float>::max_digits10);

  out << player.getX() << ' ' << player.getY() << '\n';

  out << coins << '\n';

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

  if (!(in >> magic >> version) || magic != "MYTHWOOD_SAVE" ||
      (version != 1 && version != 2)) {
    return false;
  }

  float loadedX = 0.0f;
  float loadedY = 0.0f;

  if (!(in >> loadedX >> loadedY) || !std::isfinite(loadedX) ||
      !std::isfinite(loadedY)) {
    return false;
  }

  int loadedCoins = 0;

  if (version >= 2) {
    if (!(in >> loadedCoins || loadedCoins < 0)) {
      return false;
    }
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

  const std::size_t expectedStructureCount = version == 1 ? 3 : 4;
  if (!(in >> structureCount) || structureCount != expectedStructureCount)
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

  coins = loadedCoins;

  player.setPosition(loadedX, loadedY);
  inventory.setItems(loadedItems);
  for (std::size_t i = 0; i < loadedStructureStates.size(); ++i) {
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
