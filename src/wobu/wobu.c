#include "SDL.h" // IWYU pragma: keep //clangd

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "../../external/Nuklear/nuklear.h"
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "../../external/Nuklear/demo/sdl_renderer/nuklear_sdl_renderer.h"

// #include "app.h"
#include "../core.h"

int main(int argc, char *argv[])
{
    SDL_bool running = SDL_TRUE;

    // initialize sdl
    struct core core = {
        .window_name = "WOBU", .window_width = 1200, .window_height = 800};
    if (core_setup(&core) < 0)
        core_shutdown(&core);

    // initialize nuklear
    struct nk_context *ctx;
    ctx = nk_sdl_init(core.window, core.renderer);
    {
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font *font;

        nk_sdl_font_stash_begin(&atlas);
        font = nk_font_atlas_add_default(atlas, 13, &config);
        nk_sdl_font_stash_end();
        nk_style_set_font(ctx, &font->handle);
    }

    // initialize app
    // struct app app;

    while (running) {
        /* Input */
        SDL_Event evt;
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT)
                running = 0;

            if (nk_item_is_any_active(ctx)) {
                nk_sdl_handle_event(&evt);
            } else {
                if (evt.type != SDL_MOUSEWHEEL) {
                    nk_sdl_handle_event(&evt);
                }
                // app_handle_event(&app, &evt);
            }
        }
        nk_input_end(ctx);

        /* GUI */
        // SDL_GL_GetDrawableSize(win, &app.screen_width, &app.screen_height);
        // if (!app_run(&app, ctx)) {
        //     SDL_Log("Failed to run app.");
        //     running = 0;
        // };

        /* Render */
        SDL_SetRenderDrawColor(core.renderer, 26.0f, 46.0f, 61.0f, 255.0f);
        SDL_RenderClear(core.renderer);
        // app_render(&app, renderer);
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_RenderPresent(core.renderer);
    }

    // destroy app
    nk_sdl_shutdown();
    core_shutdown(&core);
    return 0;
}
