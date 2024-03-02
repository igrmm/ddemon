#ifndef ASSETS_H
#define ASSETS_H

#include "SDL.h"

#include "../external/wobu/src/txt.h"

enum asset_texture { ASSET_TEXTURE_PLAYER = 0, ASSET_TEXTURE_MAX };
enum asset_font { ASSET_FONT_SMALL = 0, ASSET_FONT_MAX };

struct assets {
    SDL_Texture *textures[ASSET_TEXTURE_MAX];
    struct txt_font *fonts[ASSET_FONT_MAX];
};

int assets_load(struct assets *assets, SDL_Renderer *ren);
void assets_dispose(struct assets *assets);

#endif
