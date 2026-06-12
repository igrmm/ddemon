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

SDL_AppResult SDL_AppInit(void **app_state, int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    struct app *app = SDL_malloc(sizeof(struct app));
    if (app == NULL)
        return SDL_APP_FAILURE;
    *app = (struct app){0};
    *app_state = app;

    if (!core_initialize(&app->core, "DDEMON", 800, 600, SDL_WINDOW_FULLSCREEN))
        return SDL_APP_FAILURE;

    if (!assets_initialize(&app->core, &app->assets))
        return SDL_APP_FAILURE;

    if (!ui_initialize(&app->ui, app->assets.fonts[ASSET_FONT_SMALL]))
        return SDL_APP_FAILURE;

    app->win = (struct ui_element){.rect = {400, 400, 400, 400},
                                   .type = UI_TYPE_WINDOW,
                                   .data.window = {.title = "TestWindow"}};

    struct core_texture atlas_texture = atlas_get_texture(app->assets.atlas);
    core_bind_texture(&app->core, atlas_texture);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *app_state, SDL_Event *event)
{
    struct app *app = (struct app *)app_state;

    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;

    if (event->type == SDL_EVENT_WINDOW_RESIZED)
        core_update_viewport(&app->core, event->window.data1,
                             event->window.data2);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *app_state)
{
    struct app *app = (struct app *)app_state;

    count_fps();

    core_clear_screen(0.5f, 0.0f, 0.0f, 1.0f);
    SDL_FRect src_rect = {0, 0, 32, 32};
    SDL_FRect dst_rect = {0, 0, 32, 32};
    struct core_texture_region *tilemap =
        app->assets.textures[ASSET_TEXTURE_TILEMAP];

    // render some lines
    struct core_color c = {0.0f, 1.0f, 0.0f, 1.0f};
    core_add_line(&app->core, 0, 0, 100, 100, &c);
    core_add_line(&app->core, 0, 0, 100, 50, &c);

    // render 13k images on screen
    for (int l = 0; l < 6; l++) {
        for (int x = 0; x < 61; x += 1) {
            for (int y = 0; y < 35; y += 1) {
                dst_rect.x = x * 32;
                dst_rect.y = y * 32;
                src_rect.x = l * 32;
                core_add_drawing_tex(&app->core, &tilemap->rect, &src_rect,
                                     &dst_rect);
            }
        }
    }
    char text[64] = "";
    SDL_snprintf(text, SDL_arraysize(text), "THIS IS NOT A GAME. FPS=%s", fps);
    txt(text, 0, 100, app->assets.fonts[ASSET_FONT_SMALL], &app->core);
    // ui_mk_window(win, assets, ui, core); disable ui for now

    core_use_shader(&app->core, app->assets.shaders[ASSET_SHADER_DEFAULT]);
    core_render_drawings(&app->core);
    core_use_shader(&app->core, app->assets.shaders[ASSET_SHADER_LINE]);
    core_render_lines(&app->core);
    core_update_window(app->core.window);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *app_state, SDL_AppResult result)
{
    struct app *app = (struct app *)app_state;
    if (app != NULL) {
        assets_terminate(&app->assets);
        core_terminate(&app->core);
        ui_terminate(&app->ui);
        SDL_free(app);
    }

    if (result == SDL_APP_FAILURE)
        SDL_Log("SDL application ended with failure.");
}
