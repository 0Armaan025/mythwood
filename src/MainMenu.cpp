#include "../include/MainMenu.h"

#include <SDL2/SDL_render.h>
#include <iostream>
using namespace std;

namespace {
constexpr SDL_Color parchment{220, 204, 158, 255};
constexpr SDL_Color greeny_green{113, 139, 105,
                                 255}; // thanks gpt for the colors
constexpr SDL_Color dark_green{24, 35, 30, 255};

} // namespace
MainMenu::MainMenu() =
    default; // OOH YEAH I STUDIED DEFAULT CONSTRUCTORS on learncpp.com

MainMenu::~MainMenu() {
  if (titleFont)
    TTF_CloseFont(titleFont);
  if (buttonFont)
    TTF_CloseFont(buttonFont);
  TTF_Quit();
}

bool MainMenu::init() {
  if (TTF_Init() != 0) {

    cout << "ttf error: " << TTF_GetError() << std::endl;
    return false;
  }

  // I use arch btw (with hyprland and no dots, custom customization lol)
  const char *fontPath = "/usr/share/fonts/TTF/DejaVuSerif.ttf";
  titleFont = TTF_OpenFont(fontPath, 64);
  buttonFont = TTF_OpenFont(fontPath, 22);
  if (!titleFont || !buttonFont) {
    cout << "ttf error: " << TTF_GetError() << endl;
    return false;
  }

  return true;
}

// alright, the fun part:

bool MainMenu::update() {
  int mouseX, mouseY;
  const Uint32 mouse = SDL_GetMouseState(&mouseX, &mouseY);
  const bool mouseDown = mouse & SDL_BUTTON(SDL_BUTTON_LEFT);
  hoveredButton = -1;

  for (int i = 0; i < 4; ++i) {
    const auto &r = buttons[i].rect; // thanks to auto keyword otherwise it was
                                     // just an SDL_Rect but anyways

    if (mouseX >= r.x && mouseX < r.x + r.w && mouseY >= r.y &&
        mouseY < r.y + r.h) {
      hoveredButton = i;

      if (mouseDown && !mouseWasDown) {
        if (i == 3)
          return false;

        // Other buttons will get functionality later.
      }
    }
  }

  mouseWasDown = mouseDown;

  return true;
}

void MainMenu::drawText(SDL_Renderer *renderer, TTF_Font *font,
                        const std::string &text, int y, SDL_Color color) {
  SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);

  if (!surface)
    return;

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

  if (texture) {
    SDL_Rect dest{480 - surface->w / 2, y, surface->w, surface->h};

    SDL_RenderCopy(renderer, texture, nullptr, &dest);
    SDL_DestroyTexture(texture);
  }

  SDL_FreeSurface(surface);
}
void MainMenu::render(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 24, 35, 30, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 105, 119, 82, 255);

  SDL_Rect border{28, 28, 904, 584};
  SDL_RenderDrawRect(renderer, &border);

  SDL_Rect inner{36, 36, 888, 568};
  SDL_RenderDrawRect(renderer, &inner);

  drawText(renderer, titleFont, "MYTHWOOD", 115, parchment);

  drawText(renderer, buttonFont, "A forgotten world awaits", 195, greeny_green);

  SDL_Rect divider{390, 248, 180, 1};
  SDL_RenderFillRect(renderer, &divider);

  // btns
  for (int i = 0; i < 4; ++i) {
    const auto &button = buttons[i];

    SDL_SetRenderDrawColor(renderer, hoveredButton == i ? 63 : 35,
                           hoveredButton == i ? 79 : 48,
                           hoveredButton == i ? 57 : 40, 255);

    SDL_RenderFillRect(renderer, &button.rect);

    SDL_SetRenderDrawColor(renderer, 105, 119, 82, 255);
    SDL_RenderDrawRect(renderer, &button.rect);

    drawText(renderer, buttonFont, button.text, button.rect.y + 11,
             hoveredButton == i ? parchment : greeny_green);
  }

  SDL_RenderPresent(renderer);
}
