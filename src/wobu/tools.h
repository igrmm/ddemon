#ifndef TOOLS_H
#define TOOLS_H

#include "SDL.h" // IWYU pragma: keep //clangd

#include "../core.h"

enum tool_type {
    TOOL_TYPE_PENCIL = 0,
    TOOL_TYPE_ERASER,
    TOOL_TYPE_ENTITY,
    TOOL_TYPE_SELECT,
    TOOL_TYPE_TOTAL
};

struct tool {
    enum tool_type type;
    SDL_Color rect_color;
    struct core_texture icon_texture;
};

struct tool_rect {
    SDL_FRect rect;
    SDL_FPoint start;
};

struct app;
void tools_window(struct app *app);

#endif
