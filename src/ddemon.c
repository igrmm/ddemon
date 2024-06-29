#include "SDL.h" // IWYU pragma: keep //clangd

#include "assets.h"
#include "core.h"

int main(int argc, char *argv[])
{
    struct core core = {0};
    if (core_setup(&core, "DDEMON", 800, 600, SDL_WINDOW_FULLSCREEN_DESKTOP) <
        0)
        core_shutdown(&core);

    struct assets assets = {0};
    if (assets_load(&core, &assets) < 0) {
        SDL_Log("Error loading assets.");
        assets_dispose(&assets);
        core_shutdown(&core);
    }

    int frames = 0;
    Uint32 last_frame_time = 0;
    SDL_bool running = SDL_TRUE;
    while (running) {
        Uint32 now = SDL_GetTicks64();
        if (now - last_frame_time >= 1000) {
            SDL_Log("FPS: %i", frames);
            last_frame_time = now;
            frames = 0;
        }
        frames++;

        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT)
                running = SDL_FALSE;

            if (evt.type == SDL_WINDOWEVENT &&
                evt.window.event == SDL_WINDOWEVENT_RESIZED) {
                core_update_viewport(&core, evt.window.data1, evt.window.data2);
            }
        }

        core_clear_screen(0.5f, 0.0f, 0.0f, 1.0f);
        core_use_shader(&core, assets.shaders[ASSET_SHADER_DEFAULT]);

        core_bind_texture(&core, assets.atlas);

        SDL_FRect tex_region = assets.texture_regions[ASSET_TEXTURE_TILEMAP];
        SDL_FRect src_rect = {0, 0, 32, 32};
        SDL_FRect dst_rect = {0, 0, 32, 32};
        for (int l = 0; l < 6; l++) {
            for (int x = 0; x < 61; x += 1) {
                for (int y = 0; y < 35; y += 1) {
                    dst_rect.x = x * 32;
                    dst_rect.y = y * 32;
                    src_rect.x = l * 32;
                    core_add_drawing_tex(&core, &tex_region, &src_rect,
                                         &dst_rect);
                }
            }
        }

        core_draw_queue(&core);
        core_update_window(core.window);
    }

    assets_dispose(&assets);
    core_shutdown(&core);

    return 0;
}
