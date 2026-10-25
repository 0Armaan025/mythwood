cmake_minimum_required(VERSION 3.20)
project(Mythwood LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT (CMAKE_SYSTEM_NAME STREQUAL "Emscripten"))
  find_package(SDL2 REQUIRED)
  find_package(SDL2_ttf REQUIRED)
endif()

add_executable(mythwood
  src/main.cpp
  src/Game.cpp
  src/Window.cpp
  src/MainMenu.cpp
  src/World.cpp
  src/Player.cpp
  src/Inventory.cpp
  src/Structure.cpp
  src/Projectile.cpp
  src/Enemy.cpp
  src/Rocket.cpp
)

target_include_directories(mythwood PRIVATE include)

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  # Emscripten builds and installs its SDL2/SDL2_ttf ports on demand when the
  # USE_SDL/USE_SDL_TTF flags are passed (compile + link). Preload the asset
  # directory so relative file paths (fonts) resolve in the browser virtual FS.
  target_compile_options(mythwood PRIVATE -sUSE_SDL=2 -sUSE_SDL_TTF=2)
  target_link_libraries(mythwood PRIVATE -sUSE_SDL=2 -sUSE_SDL_TTF=2)
  target_link_options(mythwood PRIVATE --embed-file ${CMAKE_SOURCE_DIR}/assets/DejaVuSerif.ttf@assets/DejaVuSerif.ttf)
else()
  target_link_libraries(mythwood PRIVATE SDL2::SDL2 SDL2_ttf::SDL2_ttf)
endif()
