#ifndef UI_H
#define UI_H

#include "SDL.h" // IWYU pragma: keep //clangd

#include "assets.h"
#include "core.h"

struct ui_button {
    const char *text;
    float text_width;
    SDL_FRect tex_region;
};

struct ui_label {
    const char *text;
};

struct ui_window {
    int row_y;
    const char *title;
};

struct ui_style {
    struct core_color background_color;
    struct core_color foreground_color;
    struct core_color hover_color;
    struct core_color click_color;
    struct core_color font_color;
};

union ui_widget {
    struct ui_button button;
    struct ui_label label;
    struct ui_window window;
};

struct ui_element {
    SDL_FRect rect;
    int padding;
    struct ui_style *style;
    union ui_widget widget;
};

void ui_set_font(struct txt_font *in_font);
void ui_set_style(struct ui_style *in_style);
struct ui_style ui_get_style(void);
void ui_mk_button(struct ui_element *button, struct assets *assets,
                  struct core *core);
void ui_mk_label(struct ui_element *label, struct assets *assets,
                 struct core *core);
void ui_mk_window(struct ui_element *window, struct assets *assets,
                  struct core *core);

#endif
