#include "../include/World.h"
#include <SDL2/SDL_keyboard.h>
#include <iostream>
using namespace std;

World::~World() {
  if (font) {
    TTF_CloseFont(font);
  }
}

bool World::init() {
  font = TTF_OpenFont("/usr/share/fonts/TTF/DejaVuSerif.ttf", 20);

  if (!font) {
    cout << "ttf error: " << TTF_GetError() << endl;
    return false;
  }

  returnToMenu = false;
  return true;
}

void World::update(float deltaTime) {
  const Uint8 *keys = SDL_GetKeyboardState(nullptr);

  if (keys[SDL_SCANCODE_ESCAPE]) {
    returnToMenu = true;
    return;
  }

  player.update(deltaTime);
}

// forest time heheehehehehe

void World::render(SDL_Renderer *renderer) const {
  // ground and grasses
  SDL_SetRenderDrawColor(renderer, 35, 55, 39, 255);
  SDL_Rect ground{0, 64, 960, 576};
  SDL_RenderFillRect(renderer, &ground);

  // Simple grass patches
  SDL_SetRenderDrawColor(renderer, 42, 65, 44, 255);

  for (int i = 0; i < 30; ++i) {
    SDL_Rect patch{(i * 137 + 29) % 950, 85 + (i * 83) % 530, 22, 12};
    SDL_RenderFillRect(renderer, &patch);
  }

  // tiles/HUD

  SDL_SetRenderDrawColor(renderer, 20, 29, 25, 255);
  SDL_Rect bar{0, 0, 960, 64};
  SDL_RenderFillRect(renderer, &bar);

  SDL_SetRenderDrawColor(renderer, 105, 119, 82, 255);
  SDL_RenderDrawLine(renderer, 0, 63, 960, 63);

  SDL_Surface *surface =
      TTF_RenderUTF8_Blended(font, "MYTHWOOD", {220, 204, 158, 255});

  if (surface) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (texture) {
      SDL_Rect dest{24, 17, surface->w, surface->h};
      SDL_RenderCopy(renderer, texture, nullptr, &dest);
      SDL_DestroyTexture(texture);
    }

    SDL_FreeSurface(surface);
  }

  player.render(renderer);

  SDL_RenderPresent(renderer);
}

bool World::shouldReturnToMenu() const { return returnToMenu; }
void World::start() {
  player.reset();
  returnToMenu = false;
}
