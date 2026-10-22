#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
// TODO: i might handle text rendeirng and other stuff tom coz i have coaching
// early morning today
#include <SDL2/SDL_video.h>
#include <iostream>
using namespace std;

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {

    cout << "error: " << SDL_GetError() << endl;
    return 1;
  }

  SDL_Window *window =
      SDL_CreateWindow("Mythwood", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 960, 640, SDL_WINDOW_SHOWN);

  if (!window) {
    cout << "error: " << SDL_GetError() << endl;
    SDL_Quit();

    return 1;
  }

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (!renderer) {
    cout << "error: " << SDL_GetError() << endl;
    SDL_Quit();

    return 1;
  }

  bool running = true;
  SDL_Event event;

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }

    SDL_SetRenderDrawColor(renderer, 24, 35, 30, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
