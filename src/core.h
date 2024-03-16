/**
 *  \file core.h
 *
 *  Setup and shutdown SDL library.
 */

#ifndef CORE_H
#define CORE_H

#include "SDL.h" // IWYU pragma: keep //clangd

struct core {
    SDL_Window *window;
    SDL_Renderer *renderer;
    char window_name[100];
    int window_height;
    int window_width;
};

int core_setup(struct core *core);
void core_shutdown(struct core *core);

#endif
