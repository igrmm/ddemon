#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>

#include "arena.h"
#include "core.h"
#include "list.h"
#include "pool.h"

enum ui_type { UI_TYPE_WINDOW };

struct ui_colors {
    struct core_color background;
    struct core_color foreground;
};

struct ui_window {
    const char *title;
    float bar_height;
};

union ui_element_data {
    struct ui_window window;
};

struct ui_element {
    struct list_node node;
    enum ui_type type;
    union ui_element_data data;
    SDL_FRect rect;
    float border_thickness;
};

struct ui_poolable_element {
    struct list_node node;
    struct ui_element element;
};

struct ui {
    struct ui_poolable_element *pool;
    struct pool_handle pool_handle;
    struct ui_colors colors;
    struct txt_font *font;
    float border_thickness;
    struct list windows;
};

bool ui_initialize(struct ui *ui, struct txt_font *font, struct arena *arena);
struct ui_element *ui_create_window(float x, float y, float w, float h,
                                    const char *title, struct ui *ui);
void ui_add_drawings(struct ui *ui, struct core *core);

#endif
