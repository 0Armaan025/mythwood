// ALIRGHT WE NEED SOME UI BOYS AND GIRLS
// AND WE GOTTA WORK WITH FONT UHH I THOUGHT I'LL DO THIS TOM BUT I CAN'T RESIST
// ADDING THE UI FIRST BEFORE THE CORE GAME. SO ANYWAYS, LET'S BEGIN

#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

class MainMenu {
public:
  MainMenu();
  // this time -> constructor

  ~MainMenu(); // ew

  bool init();
  bool update();

  void render(SDL_Renderer *renderer); // reminds me of redis lol
  //

  bool takeNewGameRequest();
  bool takeContinueRequest();

private:
  TTF_Font *titleFont = nullptr;
  TTF_Font *buttonFont = nullptr;

  bool mouseWasDown =
      false; // we'll track using positions of the mouse and SDL_Rect later and
             // chedcking if the btn was prssed

  int hoveredButton = -1;

  struct Button {
    SDL_Rect rect;
    std::string text;
  };

  Button buttons[4] = {
      {{360, 290, 240, 48}, "New Game"},
      {{360, 350, 240, 48}, "Continue"},
      {{360, 410, 240, 48}, "Settings"},
      {{360, 470, 240, 48}, "Quit"},
  };

  void drawText(SDL_Renderer *renderer, TTF_Font *font, const std::string &text,
                int y, SDL_Color color);

  // not really convenient but works for now ig, coz it's not a big game like i
  // thought id' make

  bool newGameRequested = false;
  bool continueRequested = false;
};

// ARE ALL OF US RIDING TO OUR DEATHS!? YES, WE ARE!!! MY SOLIDERS SCREAM, MY
// SOLIDERS RAGE MY SOLDIERS PUSH FORWARD
