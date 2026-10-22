#pragma once

#include <SDL2/SDL.h>

class Window {
public:
  Window() = default;
  ~Window(); // i'll make this destructor thingy later coz memory management
             // sucks for me

  // for basic initalization -> gonna make stuff more structured

  bool create(const char *title, int width, int height);
  void pollEvents();
  void clear();
  void present();

  bool isRunning() const;

private:
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;
  bool running = false;
};
