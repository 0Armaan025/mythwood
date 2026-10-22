#include "../include/Window.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <iostream>
using namespace std;

Window::~Window() {
  // UHH I DON'T WANNA DO THIS, JUST GET DESTROYED destructor sucks bro

  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);

  SDL_Quit();
  // TTF_Quit later but not now
}

// wouldn't it have been better to use constructor for this stuff lol
bool Window::create(const char *title, int width, int height) {

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    cout << "error: " << SDL_GetError() << endl;
    return false;
  }

  window =
      SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       width, height, SDL_WINDOW_SHOWN);

  if (!window) {
    std::cerr << SDL_GetError() << '\n';
    return false;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (!renderer) {
    std::cerr << SDL_GetError() << '\n';
    return false;
  }

  running = true;
  return true;
}

void Window::pollEvents() {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      running = false;
    }
  }
}

void Window::clear() {
  SDL_SetRenderDrawColor(renderer, 24, 35, 30, 255);
  SDL_RenderClear(renderer);
}

void Window::present() { SDL_RenderPresent(renderer); }

bool Window::isRunning() const { return running; }
