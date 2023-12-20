#ifndef ASSETS_H
#define ASSETS_H

#include "SDL.h"

#include "../external/wobu/src/txt.h"

enum assets_texture { ASSET_TEXTURE_PLAYER = 0, NUM_TEXTURES };
enum font { ASSET_FONT_SMALL = 0, NUM_FONTS };

struct assets {
    SDL_Texture *textures[NUM_TEXTURES];
    struct txt_font *fonts[NUM_FONTS];
};

int assets_load(struct assets *assets, SDL_Renderer *ren);
void assets_dispose(struct assets *assets);

#endif
