#include "SDL.h" // IWYU pragma: keep //clangd
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "core.h"

int core_setup(struct core *core)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }

    core->window = SDL_CreateWindow(core->window_name, SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, core->window_width,
                                    core->window_height, SDL_WINDOW_RESIZABLE);
    if (core->window == NULL) {
        SDL_Log("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return -1;
    }

    core->renderer = SDL_CreateRenderer(core->window, -1, 0);
    if (core->renderer == NULL) {
        SDL_Log("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetRenderDrawBlendMode(core->renderer, SDL_BLENDMODE_BLEND);

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        SDL_Log("SDL_image could not initialize! SDL_image Error: %s\n",
                IMG_GetError());
        return -1;
    }

    if (TTF_Init() < 0) {
        SDL_Log("SDL_ttf could not initialize! SDL_ttf Error: %s\n",
                TTF_GetError());
        return -1;
    }

    return 0;
}

void core_shutdown(struct core *core)
{
    SDL_DestroyRenderer(core->renderer);
    SDL_DestroyWindow(core->window);
    SDL_Quit();
    IMG_Quit();
    TTF_Quit();
}
