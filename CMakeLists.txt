cmake_minimum_required(VERSION 3.20)
project(Mythwood LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

add_executable(mythwood
  src/main.cpp
  src/Game.cpp
  src/Window.cpp
  src/MainMenu.cpp
  src/Game.cpp
  src/World.cpp
  src/Player.cpp
  src/Inventory.cpp
  src/Structure.cpp
  src/Projectile.cpp
  src/Enemy.cpp
  src/Rocket.cpp
)

target_include_directories(mythwood PRIVATE include)
target_link_libraries(mythwood PRIVATE
  SDL2::SDL2
  SDL2_ttf::SDL2_ttf
)

