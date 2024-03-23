#include "SDL.h" // IWYU pragma: keep //clangd
#include "SDL_image.h"
#include "nk.h"

#include "../core.h"
#include "app.h"
#include "pick.h"
#include "status.h"

int app_init(struct app *app, struct core *core, struct nk_context *nk_ctx)
{
    app->core = core;
    app->nk_ctx = nk_ctx;

    SDL_Texture *tileset_texture =
        IMG_LoadTexture(core->renderer, "tileset.png");
    if (tileset_texture == NULL) {
        SDL_Log("SDL error in tileset_texture creation: %s", SDL_GetError());
        return -1;
    }
    app->tileset_texture = tileset_texture;
    app->show_pick_window = 1;
    app->window_flags = NK_WINDOW_BORDER | NK_WINDOW_SCALABLE |
                        NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE |
                        NK_WINDOW_CLOSABLE;
    return 0;
}

void app_run(struct app *app)
{
    if (app->show_pick_window)
        pick_window(app);

    status_window(app);
}

void app_shutdown(struct app *app)
{
    if (app->tileset_texture != NULL)
        SDL_DestroyTexture(app->tileset_texture);
}
