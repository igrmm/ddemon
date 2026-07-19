#include <SDL3/SDL.h>

#include "arena.h"
#include "pool.h"
#include "txt.h"
#include "ui.h"

#define UI_POOL_CAPACITY 10
#define UI_DEFAULT_BORDER_THICKNESS 1.0f
#define UI_DEFAULT_COLORS                                                      \
    (struct ui_colors)                                                         \
    {                                                                          \
        .background = {0.16f, 0.16f, 0.21f, 0.0f},                             \
        .foreground = {0.74f, 0.58f, 0.98f, 0.0f},                             \
    }

bool ui_initialize(struct ui *ui, struct txt_font *font, struct arena *arena)
{
    ui->pool = arena_alloc(arena, sizeof(struct ui_poolable_element) *
                                      UI_POOL_CAPACITY);
    if (ui->pool == NULL)
        return false;
    ui->pool_handle = (struct pool_handle){0};
    for (int i = 0; i < UI_POOL_CAPACITY; i++) {
        ui->pool[i] = (struct ui_poolable_element){0};
        pool_return(&ui->pool[i].node, &ui->pool_handle);
    }

    ui->font = font;
    ui->colors = UI_DEFAULT_COLORS;
    ui->border_thickness = UI_DEFAULT_BORDER_THICKNESS;

    return true;
}

static struct ui_element *ui_create_element(enum ui_type type, struct ui *ui)
{
    struct list_node *node = pool_obtain(&ui->pool_handle);
    if (node == NULL)
        return NULL;
    struct ui_poolable_element *poolable = (struct ui_poolable_element *)node;
    struct ui_element *element = &poolable->element;
    element->type = type;
    element->border_thickness = ui->border_thickness;
    return element;
}

struct ui_element *ui_create_window(float x, float y, float w, float h,
                                    const char *title, struct ui *ui)
{
    struct ui_element *window = ui_create_element(UI_TYPE_WINDOW, ui);
    if (window == NULL)
        return NULL;
    window->rect = (SDL_FRect){x, y, w, h};
    window->data.window.title = title;
    window->data.window.bar_height = txt_get_font_height(ui->font);
    list_add(&window->node, &ui->windows);
    return window;
}

static void ui_add_window_drawing(struct ui_element *window, struct core *core,
                                  struct ui *ui)
{
    // draw window rect with background color
    core_add_drawing_fill_rect(core, &window->rect, &ui->colors.background);

    // draw bar
    SDL_FRect bar_rect = {window->rect.x,
                          window->rect.y + window->rect.h -
                              window->data.window.bar_height,
                          window->rect.w, window->data.window.bar_height};
    core_add_drawing_fill_rect(core, &bar_rect, &ui->colors.foreground);

    // draw window border
    core_add_drawing_rect(core, &window->rect, &ui->colors.foreground,
                          window->border_thickness);
}

void ui_add_drawings(struct ui *ui, struct core *core)
{
    // iterate windows
    struct list_node *window_iterator = NULL;
    while ((window_iterator = list_iterate(window_iterator, &ui->windows))) {
        struct ui_element *window = (struct ui_element *)window_iterator;
        ui_add_window_drawing(window, core, ui);
    }
}
