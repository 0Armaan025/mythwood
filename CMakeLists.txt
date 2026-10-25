cmake_minimum_required(VERSION 3.20)
project(Mythwood LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten" OR CMAKE_TOOLCHAIN_FILE MATCHES "Emscripten")
  # Emscripten: SDL2/SDL2_ttf come from emscripten ports via -sUSE_SDL flags.
  # Font is preloaded into the virtual fs at root path (DejaVuSerif.ttf).
  # ALLOW_MEMORY_GROWTH lets the heap expand past the default 16 MB cap.
  target_compile_options(mythwood PRIVATE -sUSE_SDL=2 -sUSE_SDL_TTF=2)
  target_link_libraries(mythwood PRIVATE -sUSE_SDL=2 -sUSE_SDL_TTF=2)
  target_link_options(mythwood PRIVATE
    --preload-file ${CMAKE_SOURCE_DIR}/assets/DejaVuSerif.ttf@DejaVuSerif.ttf
    -sALLOW_MEMORY_GROWTH=1
    -sINITIAL_MEMORY=33554432
  )
else()
  find_package(SDL2 REQUIRED)
  find_package(SDL2_ttf REQUIRED)
  target_link_libraries(mythwood PRIVATE SDL2::SDL2 SDL2_ttf::SDL2_ttf)
endif()
