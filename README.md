# mythwood

a small top-down exploration game made with C++ and SDL2.

test it here: https://beautiful-panda-48426d.netlify.app/

## how it works

you spawn in a forest and walk around with WASD or arrow keys.

- **E** - gather resources / interact with things
- **I** - open or close inventory
- **SPACE** - melee attack
- **P** - equip or unequip the moonfire sigil (gun)

there are slimes to fight, trees and rocks to harvest, structures to find, and a shop where you can buy stuff with coins you get from selling wood and stone. if you get the rocket launcher and finish the launcher ammo, you reach the ending screen.

the main menu lets you start a new game, continue a save, read how to play, or quit.

## clone and run (desktop)

you need SDL2 and SDL2_ttf installed.

```bash
git clone https://github.com/0Armaan025/mythwood.git
cd mythwood
cmake -B build -S .
cmake --build build
./build/mythwood
```

on arch that's `sudo pacman -S sdl2 sdl2_ttf`.

## run on web

the web build uses emscripten. install the emsdk, then:

```bash
source ~/.emsdk/emsdk_env.sh
emcmake cmake -B build-web -S .
cmake --build build-web
```

then serve the `build-web/` folder over http (don't open index.html directly, the wasm won't load):

```bash
python3 -m http.server 8000 -d build-web
```

open `http://localhost:8000`.

the font is embedded in the build so it works in the browser.

## branches

- `main` - desktop build
- `web-deployment` - emscripten / web build

give it a star if you liked it.
