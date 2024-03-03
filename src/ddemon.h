#ifndef DDEMON_H
#define DDEMON_H

#include "SDL.h"

#include "assets.h"

struct ddemon {
    SDL_Window *window;
    SDL_Renderer *renderer;
    struct assets assets;
};

#endif
