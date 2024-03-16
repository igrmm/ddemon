#include "SDL.h" // IWYU pragma: keep //clangd

#include "../external/wobu/src/txt.h"

#include "assets.h"
#include "core.h"
#include "ddemon.h"

int main(int argc, char *argv[])
{
    SDL_bool running = SDL_TRUE;
    struct ddemon game = {.core = {.window_name = "DDEMON",
                                   .window_width = 1920,
                                   .window_height = 1080}};
    if (core_setup(&game.core) < 0)
        core_shutdown(&game.core);

    SDL_Renderer *ren = game.core.renderer;
    struct assets *assets = &game.assets;

    if (assets_load(assets, ren) < 0) {
        SDL_Log("Error loading assets.");
        assets_dispose(assets);
        core_shutdown(&game.core);
    }

    int frames = 0;
    Uint32 last_frame_time = 0;
    char fps[512];

    SDL_Texture *tex = assets->textures[ASSET_TEXTURE_PLAYER];

    struct txt_font *font = assets->fonts[ASSET_FONT_SMALL];

    while (running) {
        Uint32 now = SDL_GetTicks64();
        if (now - last_frame_time >= 1000) {
            SDL_snprintf(fps, sizeof(fps), "fps: %i", frames);
            last_frame_time = now;
            frames = 0;
        }
        frames++;

        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT)
                running = SDL_FALSE;
        }
        SDL_RenderClear(ren);
        SDL_Rect dstrect = {0, 0, 32, 32};
        for (int l = 0; l < 6; l++) {
            for (int x = 0; x < 1920; x += 32) {
                for (int y = 0; y < 1080; y += 32) {
                    dstrect.x = x;
                    dstrect.y = y;
                    SDL_RenderCopy(ren, tex, NULL, &dstrect);
                }
            }
        }

        txt("HELLO WORLD", 0, 0, ren, font);
        txt(fps, 0, 30, ren, font);

        SDL_RenderPresent(ren);
    }

    assets_dispose(assets);
    core_shutdown(&game.core);

    return 0;
}
