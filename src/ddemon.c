#include "SDL.h" // IWYU pragma: keep //clangd

#include "assets.h"
#include "core.h"

int main(int argc, char *argv[])
{
    if (core_setup() < 0)
        core_shutdown();

    struct assets assets = {0};
    if (assets_load(&assets) < 0) {
        SDL_Log("Error loading assets.");
        assets_dispose(&assets);
        core_shutdown();
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
        }

        core_clear_screen(0.5f, 0.0f, 0.0f, 1.0f);
        core_use_shader(assets.shaders[ASSET_SHADER_DEFAULT]);
        core_draw_map(assets.textures[ASSET_TEXTURE_TILEMAP]);
        core_update_window();
    }

    assets_dispose(&assets);
    core_shutdown();

    return 0;
}
