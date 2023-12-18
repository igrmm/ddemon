cmake_minimum_required(VERSION 3.5)
project(isgt)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(external/SDL EXCLUDE_FROM_ALL)
add_subdirectory(external/SDL_image EXCLUDE_FROM_ALL)
add_subdirectory(external/SDL_ttf EXCLUDE_FROM_ALL)

add_executable(${PROJECT_NAME}
    WIN32
    src/main.c
    external/wobu/src/txt.c
    )

if(MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE mingw32)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL2::SDL2main
    SDL2::SDL2
    SDL2_image
    SDL2_ttf
    )
