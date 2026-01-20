cmake_minimum_required(VERSION 3.28 FATAL_ERROR)
project(dmg LANGUAGES CXX)

find_package(SDL2 REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

add_executable(dmg src/main.cpp src/sm83.cpp src/ppu.cpp src/apu.cpp src/cart.cpp src/dmg.cpp)
target_include_directories(dmg PUBLIC include)
target_link_libraries(dmg PRIVATE SDL2::SDL2)
