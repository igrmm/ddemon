#ifndef APP_H
#define APP_H

#include "SDL.h" // IWYU pragma: keep //clangd
#include "nk.h"

#include "map.h"
#include "work.h"

#define GREY                                                                   \
    (struct nk_color) { 45, 45, 45, 255 }
#define GREEN                                                                  \
    (struct nk_color) { 0, 200, 150, 255 }
#define RED                                                                    \
    (struct nk_color) { 128, 0, 0, 255 }
#define WHITE                                                                  \
    (struct nk_color) { 255, 255, 255, 255 }

struct app {
    struct map *map;
    struct ecs *ecs;
    struct ecs_table *selected_entities;
    struct core *core;
    struct nk_context *nk_ctx;
    struct work work;
    SDL_Texture *tileset_texture;
    SDL_Point selected_tileset_index;
    int show_pick_window, show_grid, show_tool_window, show_properties_window;
    int window_flags;
};

int app_init(struct app *app, struct core *core, struct nk_context *nk_ctx);
void app_handle_event(struct app *app, SDL_Event *event);
void app_run(struct app *app);
void app_render(struct app *app);
void app_shutdown(struct app *app);

#endif
