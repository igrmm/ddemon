#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "SDL.h" // IWYU pragma: keep //clangd

enum component_type { CMP_TYPE_RECT = 0, CMP_TYPE_RENDERABLE, CMP_TYPE_TOTAL };

struct component_rect {
    SDL_FRect rect;
};

struct component_renderable {
    SDL_Texture *texture;
};

union component_data {
    struct component_rect rect;
    struct component_renderable renderable;
};

struct component {
    enum component_type type;
    Uint32 entity;
    SDL_bool alive;
    struct component *next;
    struct component *prev;
    union component_data data;
};

#endif
