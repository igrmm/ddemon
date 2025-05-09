#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>

#include "assets.h"
#include "core.h"
#include "queue.h"

enum ui_type { UI_TYPE_BUTTON, UI_TYPE_LABEL, UI_TYPE_WINDOW };

struct ui_button {
    const char *text;
    int text_width;
    SDL_FRect tex_region;
};

struct ui_label {
    const char *text;
    int text_width;
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

union ui_element_data {
    struct ui_button button;
    struct ui_label label;
    struct ui_window window;
};

struct ui_element {
    SDL_FRect rect;
    int padding;
    struct ui_style *style;
    enum ui_type type;
    union ui_element_data data;
};

struct ui {
    struct ui_element *element_queue;
    struct queue_handle element_queue_handle;
    struct txt_font *font;
    struct ui_style style;
};

bool ui_initialize(struct ui *ui, struct txt_font *font);

/**
 * Layout ui elements horizontally.
 *
 * Layout ui elements horizontally with a maximum height of given height. Null
 * pointers that are part of elements array will be treated as growable empty
 * spaces.
 *
 */
void ui_layout_row(struct ui_element *window, int height,
                   struct ui_element *elements[], int element_count);

void ui_mk_button(struct ui_element *button, struct assets *assets,
                  struct ui *ui, struct core *core);
void ui_mk_label(struct ui_element *label, struct assets *assets, struct ui *ui,
                 struct core *core);
void ui_mk_window(struct ui_element *window, struct assets *assets,
                  struct ui *ui, struct core *core);
void ui_terminate(struct ui *ui);

#endif
