cmake_minimum_required(VERSION 3.20)
project(Mythwood LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

add_executable(mythwood
  src/main.cpp
  src/Game.cpp
  src/Window.cpp
)

target_include_directories(mythwood PRIVATE include)
target_link_libraries(mythwood PRIVATE SDL2::SDL2)

