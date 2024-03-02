#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "../external/wobu/src/txt.h"

#include "assets.h"

// clang-format off
static const char *TEXTURE_PATHS[] = {
    [ASSET_TEXTURE_PLAYER] = "img.png",
    [ASSET_TEXTURE_MAX] = 0
};

static const char FONT_PATH[] = "NotoSansMono-Regular.ttf";
// clang-format on

int assets_load(struct assets *assets, SDL_Renderer *ren)
{
    for (int i = 0; i < ASSET_TEXTURE_MAX; i++) {
        assets->textures[i] = IMG_LoadTexture(ren, TEXTURE_PATHS[i]);
        if (assets->textures[i] == NULL) {
            SDL_Log("Error loading texture: %s\n", SDL_GetError());
            return -1;
        }
    }

    TTF_Font *ttf_small = TTF_OpenFont(FONT_PATH, 26);
    if (ttf_small == NULL) {
        SDL_Log("Error loading ttf font! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }
    struct txt_codepoint_cache *cache = txt_create_codepoint_cache();
    if (cache == NULL) {
        TTF_CloseFont(ttf_small);
        SDL_Log("Error creating txt_codepoint_cache.");
        return -1;
    }
    // cache all ASCII table
    char str[2] = {0};
    for (char c = ' '; c <= '~'; c++) {
        SDL_snprintf(str, 2, "%c", c);
        txt_cache_codepoint(cache, str);
    }
    assets->fonts[ASSET_FONT_SMALL] = txt_create_font(cache, ttf_small, ren);
    TTF_CloseFont(ttf_small);
    SDL_free(cache);
    if (assets->fonts[ASSET_FONT_SMALL] == NULL) {
        SDL_Log("Error creating txt_font.");
        return -1;
    }

    return 0;
}

void assets_dispose(struct assets *assets)
{
    for (int i = 0; i < ASSET_TEXTURE_MAX; i++) {
        if (assets->textures[i] != NULL)
            SDL_DestroyTexture(assets->textures[i]);
    }

    for (int i = 0; i < ASSET_FONT_MAX; i++) {
        if (assets->fonts[i] != NULL)
            txt_destroy_font(assets->fonts[i]);
    }
}
