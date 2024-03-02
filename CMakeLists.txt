cmake_minimum_required(VERSION 3.5)
project(ddemon)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2_image REQUIRED CONFIG)
find_package(SDL2_ttf REQUIRED CONFIG)

add_executable(${PROJECT_NAME}
    WIN32
    src/ddemon.c
    src/assets.c
    external/wobu/src/txt.c
    )

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL2::SDL2main
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
    )
