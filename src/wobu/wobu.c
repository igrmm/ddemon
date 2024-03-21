#include "SDL.h" // IWYU pragma: keep //clangd

#define NK_IMPLEMENTATION
#include "nk.h"
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "../../external/Nuklear/demo/sdl_renderer/nuklear_sdl_renderer.h"

#include "../core.h"
#include "app.h"
#include "windows.h"

int main(int argc, char *argv[])
{
    SDL_bool running = SDL_TRUE;

    // initialize sdl
    struct core core = {
        .window_name = "WOBU", .window_width = 1200, .window_height = 800};
    if (core_setup(&core) < 0)
        core_shutdown(&core);

    // initialize nuklear
    struct nk_context *nk_ctx;
    nk_ctx = nk_sdl_init(core.window, core.renderer);
    {
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font *font;

        nk_sdl_font_stash_begin(&atlas);
        font = nk_font_atlas_add_default(atlas, 13, &config);
        nk_sdl_font_stash_end();
        nk_style_set_font(nk_ctx, &font->handle);
    }

    //  initialize app
    struct app app = {0};
    app_init(&app, &core, nk_ctx);

    while (running) {
        /* Input */
        SDL_Event evt;
        nk_input_begin(nk_ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT)
                running = 0;

            if (evt.type == SDL_WINDOWEVENT) {
                if (evt.window.event == SDL_WINDOWEVENT_RESIZED ||
                    evt.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    SDL_GetWindowSize(core.window, &core.window_width,
                                      &core.window_height);
                }
            }

            if (nk_item_is_any_active(nk_ctx)) {
                nk_sdl_handle_event(&evt);
            } else {
                if (evt.type != SDL_MOUSEWHEEL) {
                    nk_sdl_handle_event(&evt);
                }
                // app_handle_event(&app, &evt);
            }
        }
        nk_input_end(nk_ctx);

        /* GUI */
        app_run(&app);

        /* Render */
        SDL_SetRenderDrawColor(core.renderer, 26.0f, 46.0f, 61.0f, 255.0f);
        SDL_RenderClear(core.renderer);
        // app_render(&app);
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_RenderPresent(core.renderer);
    }

    // destroy app
    nk_sdl_shutdown();
    core_shutdown(&core);
    return 0;
}
