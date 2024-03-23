#ifndef APP_H
#define APP_H

#include "SDL.h" // IWYU pragma: keep //clangd
#include "nk.h"

#define GREY                                                                   \
    (struct nk_color) { 45, 45, 45, 255 }
#define GREEN                                                                  \
    (struct nk_color) { 0, 200, 150, 255 }
#define RED                                                                    \
    (struct nk_color) { 128, 0, 0, 255 }
#define WHITE                                                                  \
    (struct nk_color) { 255, 255, 255, 255 }

struct app {
    struct core *core;
    struct nk_context *nk_ctx;
    SDL_Texture *tileset_texture;
    SDL_Point selected_tileset_index;
    int show_tilesetw;
    int window_flags;
};

int app_init(struct app *app, struct core *core, struct nk_context *nk_ctx);
void app_run(struct app *app);
void app_shutdown(struct app *app);

#endif
