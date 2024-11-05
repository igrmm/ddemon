#ifndef UI_H
#define UI_H

#include "SDL.h" // IWYU pragma: keep //clangd

#include "assets.h"
#include "core.h"

struct ui_window {
    struct core_color bg_color, fg_color;
    SDL_FRect rect;
    const char *title;
};

void ui_mk_window(struct ui_window *ui_window, struct assets *assets,
                  struct core *core);

#endif
