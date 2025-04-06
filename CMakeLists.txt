cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(CHIP-8_EMU)
find_package(SDL2 REQUIRED)
add_executable(main src/main.cpp src/chip8.cpp)
target_include_directories(main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(main SDL2main SDL2)