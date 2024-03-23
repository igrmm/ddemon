cmake_minimum_required(VERSION 3.5)
project(wobu)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2_image REQUIRED CONFIG)
find_package(SDL2_ttf REQUIRED CONFIG)

add_executable(${PROJECT_NAME}
    WIN32
    wobu.c
    app.c
    window_status.c
    window_tileset.c
    ../core.c
    )

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    if (DEFINED ENV{SANITIZERS})
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} $ENV{SANITIZERS}")
    endif()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror=vla)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    m
    SDL2::SDL2main
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
    )
