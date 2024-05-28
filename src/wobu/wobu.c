#include "../../external/glad.h"

#include "SDL.h" // IWYU pragma: keep //clangd

#define NK_IMPLEMENTATION
#include "nk.h"
#define NK_SDL_GL3_IMPLEMENTATION
#include "../../external/Nuklear/demo/sdl_opengl3/nuklear_sdl_gl3.h"

#include "../assets.h"
#include "../core.h"
#include "app.h"

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

int main(int argc, char *argv[])
{
    SDL_bool running = SDL_TRUE;

    // initialize sdl
    struct core core = {0};
    if (core_setup(&core, "WOBU", 1200, 800, 0) < 0)
        core_shutdown(&core);

    struct assets assets = {0};
    if (assets_load(&assets) < 0) {
        SDL_Log("Error loading assets.");
        assets_dispose(&assets);
        core_shutdown(&core);
    }

    // initialize nuklear
    struct nk_context *nk_ctx;
    nk_ctx = nk_sdl_init(core.window);
    {
        struct nk_font_atlas *nk_font_atlas;
        nk_sdl_font_stash_begin(&nk_font_atlas);
        nk_sdl_font_stash_end();
    }

    //  initialize app
    struct app app = {0};
    if (app_init(&app, &core, &assets, nk_ctx) < 0) {
        nk_sdl_shutdown();
        core_shutdown(&core);
        app_shutdown(&app);
        return -1;
    }

    while (running) {
        /* Input */
        SDL_Event evt;
        nk_input_begin(nk_ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT)
                running = 0;

            if (evt.type == SDL_WINDOWEVENT &&
                evt.window.event == SDL_WINDOWEVENT_RESIZED) {
                core_update_viewport(&core, evt.window.data1, evt.window.data2);
            }

            if (nk_item_is_any_active(nk_ctx)) {
                nk_sdl_handle_event(&evt);
            } else {
                if (evt.type != SDL_MOUSEWHEEL) {
                    nk_sdl_handle_event(&evt);
                }
                app_handle_event(&app, &evt);
            }
        }
        nk_input_end(nk_ctx);

        /* GUI */
        app_run(&app);

        /* Render */
        core_clear_screen(0.1f, 0.18f, 0.24, 1.0f);
        app_render(&app);
        nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY,
                      MAX_ELEMENT_MEMORY);
        core_update_window(core.window);
    }

    nk_sdl_shutdown();
    core_shutdown(&core);
    app_shutdown(&app);
    return 0;
}
