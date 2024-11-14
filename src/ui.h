#ifndef UI_H
#define UI_H

#include "SDL.h" // IWYU pragma: keep //clangd

#include "assets.h"
#include "core.h"

struct ui_label {
    struct core_color color;
    int padding;
    SDL_FRect rect;
    const char *text;
};

struct ui_window {
    struct core_color bg_color, fg_color;
    SDL_FRect rect;
    int row_y;
    const char *title;
};

void ui_mk_label(struct ui_label *label, struct assets *assets,
                 struct core *core);
void ui_mk_window(struct ui_window *window, struct assets *assets,
                  struct core *core);

#endif
