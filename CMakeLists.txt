cmake_minimum_required(VERSION 3.31)
project(ddemon)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

add_executable(${PROJECT_NAME}
    WIN32
    src/ddemon.c
    src/assets.c
    src/atlas.c
    src/ecs.c
    src/core.c
    src/txt.c
    src/ui.c
    src/queue.c
    )

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    if (DEFINED ENV{SANITIZERS})
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} $ENV{SANITIZERS}")
    endif()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror=vla -Wstack-usage=1024)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    m
    SDL3::SDL3
    )
