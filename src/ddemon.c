#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "assets.h"
#include "atlas.h"
#include "core.h"
#include "txt.h"
#include "ui.h"

struct app {
    struct core core;
    struct assets assets;
    struct ui ui;
    struct ui_element win;
};

static char fps[12];
void count_fps(void)
{
    static Uint64 timer;
    static int frames;
    Uint64 now = SDL_GetTicks();
    if (now - timer >= 1000) {
        SDL_snprintf(fps, SDL_arraysize(fps), "%i", frames);
        timer = now;
        frames = 0;
    }
    frames++;
}

SDL_AppResult SDL_AppInit(void **app, int argc, char *argv[])
{
    *app = SDL_calloc(1, sizeof(struct app));
    if (*app == NULL)
        return SDL_APP_FAILURE;

    struct core *core = &((struct app *)*app)->core;
    struct assets *assets = &((struct app *)*app)->assets;
    struct ui *ui = &((struct app *)*app)->ui;
    struct ui_element *win = &((struct app *)*app)->win;

    if (!core_setup(core, "DDEMON", 800, 600, SDL_WINDOW_FULLSCREEN))
        return SDL_APP_FAILURE;

    if (!assets_load(core, assets))
        return SDL_APP_FAILURE;

    if (!ui_initialize(ui, assets->fonts[ASSET_FONT_SMALL]))
        return SDL_APP_FAILURE;

    *win = (struct ui_element){.rect = {400, 400, 400, 400},
                               .type = UI_TYPE_WINDOW,
                               .data.window = {.title = "TestWindow"}};

    struct core_texture atlas_texture = atlas_get_texture(assets->atlas);
    core_bind_texture(core, atlas_texture);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *app, SDL_Event *event)
{
    struct core *core = &((struct app *)app)->core;

    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;

    if (event->type == SDL_EVENT_WINDOW_RESIZED)
        core_update_viewport(core, event->window.data1, event->window.data2);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *app)
{
    struct core *core = &((struct app *)app)->core;
    struct assets *assets = &((struct app *)app)->assets;
    struct ui *ui = &((struct app *)app)->ui;
    struct ui_element *win = &((struct app *)app)->win;

    count_fps();

    core_clear_screen(0.5f, 0.0f, 0.0f, 1.0f);
    SDL_FRect src_rect = {0, 0, 32, 32};
    SDL_FRect dst_rect = {0, 0, 32, 32};
    SDL_FRect tex_region;
    assets_get_texture_region(assets, ASSET_TEXTURE_TILEMAP, &tex_region);

    // render some lines
    struct core_color c = {0.0f, 1.0f, 0.0f, 1.0f};
    core_add_line(core, 0, 0, 100, 100, &c);
    core_add_line(core, 0, 0, 100, 50, &c);

    // render 13k images on screen
    for (int l = 0; l < 6; l++) {
        for (int x = 0; x < 61; x += 1) {
            for (int y = 0; y < 35; y += 1) {
                dst_rect.x = x * 32;
                dst_rect.y = y * 32;
                src_rect.x = l * 32;
                core_add_drawing_tex(core, &tex_region, &src_rect, &dst_rect);
            }
        }
    }
    char text[64] = "";
    SDL_snprintf(text, SDL_arraysize(text), "THIS IS NOT A GAME. FPS=%s", fps);
    txt(text, 0, 100, assets->fonts[ASSET_FONT_SMALL], core);
    ui_mk_window(win, assets, ui, core);

    core_use_shader(core, assets->shaders[ASSET_SHADER_DEFAULT]);
    core_render_drawings(core);
    core_use_shader(core, assets->shaders[ASSET_SHADER_LINE]);
    core_render_lines(core);
    core_update_window(core->window);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *app, SDL_AppResult result)
{
    if (app != NULL) {
        struct assets *assets = &((struct app *)app)->assets;
        struct core *core = &((struct app *)app)->core;
        struct ui *ui = &((struct app *)app)->ui;
        assets_dispose(assets);
        core_shutdown(core);
        ui_terminate(ui);
        SDL_free(app);
    }
}
